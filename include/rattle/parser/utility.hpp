#pragma once

#include <cassert>
#include <cstddef>
#include <utility>

namespace rattle::parser::internal {
  // Small stack that uses node blocks that are cached
  // to reduce memory foot print. Mainly for storing all
  // Nested blocks encountered.
  template <class T, std::size_t N, class A> class Stack {
    static_assert(N != 0, "N cannot be 0");
    struct Node;
    std::size_t position;
    Node *list;
    A &allocator;

    // Allocate and initialize a node block
    Node *allocate_node() {
      void *slab = allocator.allocate(sizeof(Node));
      new (slab) Node(); // default construct the Node
      return static_cast<Node *>(slab);
    }

    // Prepares for push
    void ensure_slot() {
      // Moves the top pointer to the next
      // uninitialized memory slot, ensuring the slot
      // exists
      if (list == nullptr) {
        // Find memory and initialize
        list = allocate_node();
        position = 0;
      } else if (position == N - 1) {
        if (list->next == nullptr) {
          // Allocate new bunch to
          list->next = allocate_node();
          list->next->prev = list;
          list = list->next;
        } else {
          // Use cached memory
          list = list->next;
        }
        position = 0;
      }
    }

    // Pops the top element
    void pop_top_slot() {
      assert(position != std::size_t(-1));
      // Properly destroy the top element
      get_top_ptr()->~T();
      if (position == 0) {
        if (list->prev == nullptr) {
          position = std::size_t(-1);
        } else {
          list = list->prev;
          position = N - 1;
        }
      } else {
        position--;
      }
    }

    // Get the pointer of the item at the top of the stack
    T *get_top_ptr() {
      assert(list != nullptr and position != size_t(-1));
      return &static_cast<T *>(static_cast<void *>(list->buffer))[ position ];
    }

  public:
    Stack(A &allocator)
      : position{std::size_t(-1)}, list{nullptr}, allocator{allocator} {}
    bool empty() const { return position == std::size_t(-1); }
    // We can ignore empty pops, but is that okay? I guess.
    void pop() {
      if (not empty()) {
        pop_top_slot();
      }
    }
    // Get the reference to the item on top of the stack
    T &top() {
      assert(not empty());
      return *get_top_ptr();
    }
    // Push an item onto the stack
    void push(T t) {
      ensure_slot();
      // Call the move constructor on the top pointer of the stack
      new (get_top_ptr()) T(std::move(t));
    }
  };

  template <class T, std::size_t N, class A> struct Stack<T, N, A>::Node {
    Node(): prev{nullptr}, next{nullptr}, buffer{} {}
    // Help in caching since we only allocate, worst case would be a long path
    // that is only utilised once which we have know way to optimize now.
    // Doubly linked list of node blocks.
    Node *prev, *next;
    // We use a char buffer to prevent T's default constructor being called on
    // the array, if it has side effects then we instantly have a full stack
    // but incrementally constructing using placement new fixes that.
    char buffer[ sizeof(T) * N ];
  };

  // Track how many scopes entered.
  class Scopes {
    unsigned braces, parens, brackets;

  public:
    Scopes(): braces{0}, parens{0}, brackets{0} {}
    // Decrement RAII
    class Decrementor {
      unsigned &value;

    public:
      Decrementor(unsigned &v): value{v} {}
      ~Decrementor() { value--; }
    };
    // Enter a new scope and let RAII handle exiting
    Decrementor enter_paren() { return ++parens; }
    Decrementor enter_bracket() { return ++brackets; }
    // Block handling in a pipeline doesn't fit well with RAII
    void enter_scope() { braces++; }
    void leave_scope() { braces--; }
    // Check if we are in the respective scopes; unsigned is convertible to bool
    unsigned in_paren() const { return parens; }
    unsigned in_bracket() const { return brackets; }
    unsigned in_scope() { return braces; }
  };
} // namespace rattle::parser::internal


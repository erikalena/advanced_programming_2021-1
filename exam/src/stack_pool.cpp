#include <iostream>
#include <vector>
#include <cassert>
#include <algorithm>
#include <chrono>
#include <iterator>

struct index_invalid {
  std::string message;
  index_invalid(std::string s) : message{std::move(s)} {}
};

enum class method { push, pop };


template <typename stack_pool, typename T, typename N = std::size_t>
class _iterator {
  stack_pool* p;
  N stack_index;
public:
    using value_type = T;
    using reference = value_type&;
    using pointer = value_type*;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::forward_iterator_tag;
    
    explicit _iterator(stack_pool* pool, N x) : p{pool}, stack_index{x} {}
    reference operator*() const { return p->value(stack_index); }

    _iterator& operator++() {
      stack_index = p->next(stack_index);
      return *this;
    }

    _iterator operator++(int) {
      auto tmp = *this;
      ++(*this);
      return tmp;
    }

    friend bool operator==(const _iterator& x, const _iterator& y) {
      return (x.stack_index == y.stack_index );
    }

    friend bool operator!=(const _iterator& x, const _iterator& y) {
      return !(x==y); 
    }
}; 

template <typename T, typename N = std::size_t>
class stack_pool{
  struct node_t{
    T value;
    N next;
  };
  std::vector<node_t> pool;
  using stack_type = N;
  using value_type = T;
  using size_type = typename std::vector<node_t>::size_type;
  stack_type free_nodes; // at the beginning, it is empty
  
  node_t& node(stack_type x) noexcept { return pool[x-1]; } // node which has index 1 is actually stored at position zero and so on
  const node_t& node(stack_type x) const noexcept { return pool[x-1]; }

  public:
  stack_pool() : free_nodes{end()} {};
  explicit stack_pool(size_type n) : free_nodes{0} { pool.reserve(n); }; // reserve n nodes in the pool
   ~stack_pool() = default;

  using iterator = _iterator<stack_pool, value_type, stack_type>;
  using const_iterator = _iterator<const stack_pool, const value_type, stack_type>;

  iterator begin(stack_type x) { return iterator{this, x}; };
  iterator end(stack_type ) { return iterator{this, end()}; }; 
    
  const_iterator begin(stack_type x) const { return const_iterator{this,x}; };
  const_iterator end(stack_type ) const { return const_iterator{this, end()}; }; 
  
  const_iterator cbegin(stack_type x) const {  return const_iterator{this,x}; };
  const_iterator cend(stack_type ) const {  return const_iterator{this,end()}; };
    
  stack_type new_stack()  noexcept { stack_type new_stack{0}; return new_stack; };  // return an empty stack

  void reserve(size_type n) { pool.reserve(n); }; // reserve n nodes in the pool
  size_type capacity() const noexcept { return pool.capacity(); }; // the capacity of the pool

  /* 
     check if the stack taken as input is empty by 
     comparing top index and end of the stack, which 
     is always zero
  */
  bool empty(stack_type x) const noexcept { if( x == end()) return true; else return 0;}; 

  stack_type end() const noexcept { return stack_type(0); }

  /*
    access the inner value of node at the given index
  */
  T& value(stack_type x)  noexcept { return node(x).value; };
  const T& value(stack_type x) const noexcept { return node(x).value; };
  
  /*
    the following functions are to obtain the index of next node in the stack
  */
  stack_type& next(stack_type x)  noexcept  { return node(x).next;};
  const stack_type& next(stack_type x) const  noexcept { return node(x).next;};

   /*
    the following functions insert a new node on the top of the stack
    taken as input. 
    the functions do not control if the index provided is the head of 
    one stack, since doing this would mean to increase class complexity.
  */
  stack_type push(const T& val, stack_type head)  { return _push(val, head); }; 
  stack_type push(T&& val, stack_type head) { return _push(std::move(val),head);};
  

  /* 
    this function removes node on the top of a stack and add it to 
    the "stack" of free nodes by using auxiliary function 'free_node'.
    A very simple control is made to check if stack is not empty.
  */
    stack_type pop(stack_type x) noexcept { 
        try {
            check_index(x, method::pop);
        } catch (const index_invalid& s) {
            std::cerr << s.message << std::endl;
            return 1;
        }
        stack_type head = x;
        if(!empty(x)) { 
          head = next(x);
          free_nodes = free_node(x, free_nodes);
        }
        return head;
    }; 
    stack_type free_node(stack_type x, stack_type free)  noexcept {
        stack_type tmp = free;
        free = x;
        next(free) = tmp;
        return free;
    };

  /* 
    this function move all nodes in the given stack to the list
    of free nodes by simply swapping their indexes.
  */
    stack_type free_stack(stack_type x)noexcept { 
        const stack_type start = x;
        const stack_type next_idx = next(x);
        for (auto it = begin(next_idx); it != end(0); it++) {
            x = next(x);
        }
        next(x) = free_nodes;
        free_nodes = start;
        return end();
    }; 

    void check_index(stack_type x, method m) {
        switch(m) {
            case method::push:
                if (x < end())
                    throw index_invalid{
                        "You provided an invalid index: " + std::to_string(x) };
                    break;
            case method::pop:
                if (x <= end())
                    throw index_invalid{
                    "You provided an invalid index: " + std::to_string(x) };
                break;
            default:
                  std::cerr << "unknown insertion method" << std::endl;
        }
    };

    void print_stack (stack_type x) noexcept {
    for (auto it = begin(x); it != end(0); it++) 
        std::cout << *it << ' ';
    std::cout <<std::endl;
    } ; 

    private:

        template <typename D>
        stack_type _push(D&& val, stack_type head) {
            try {
                check_index(head, method::push);
            } catch (const index_invalid& s) {
                std::cerr << s.message << std::endl;
                return 1;
            }
            if(!empty(free_nodes)) { 
                stack_type tmp = free_nodes;
                free_nodes = next(free_nodes);
                value(tmp) = std::forward<D>(val);
                next(tmp) = head;
                return tmp;
            }
            else  {
                pool.push_back({std::forward<D>(val),head});
                return pool.size();
            }
        }

};

int main() {
    stack_pool<int> pool{22};
    auto l1 = pool.new_stack();
    // credits: pi as random number generator :)
    l1 = pool.push(3, l1);
    l1 = pool.push(1, l1);
    l1 = pool.push(4, l1);
    l1 = pool.push(1, l1);
    l1 = pool.push(5, l1);
    l1 = pool.push(9, l1);
    l1 = pool.push(2, l1);
    l1 = pool.push(6, l1);
    l1 = pool.push(5, l1);
    l1 = pool.push(3, l1);
    l1 = pool.push(5, l1);
    
    auto t_start = std::chrono::high_resolution_clock::now();
    pool.print_stack(l1);
    auto t_end = std::chrono::high_resolution_clock::now();

    auto t = std::chrono::duration_cast<std::chrono::nanoseconds>(t_end - t_start);
    std::cout << t.count() << std::endl;

    auto l2 = pool.new_stack();
    l2 = pool.push(8, l2);
    l2 = pool.push(9, l2);
    l2 = pool.push(7, l2);
    l2 = pool.push(9, l2);
    l2 = pool.push(3, l2);
    l2 = pool.push(1, l2);
    l2 = pool.push(1, l2);
    l2 = pool.push(5, l2);
    l2 = pool.push(9, l2);
    l2 = pool.push(9, l2);
    l2 = pool.push(7, l2);

    auto M = std::max_element(pool.begin(l1), pool.end(l1));
    assert(*M == 9);

    auto m = std::min_element(pool.begin(l2), pool.end(l2));
    assert(*m == 1);

}
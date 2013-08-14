#ifndef SHADOWRUN5_COMBAT_MENU_HPP
#define SHADOWRUN5_COMBAT_MENU_HPP

#include <ncurses.h>
#include <menu.h>
#include <vector>
#include <list>
#include <string>
#include <initializer_list>

#include "window.hpp"
#include "error.hpp"

struct default_item
{
  using item_ptr = std::unique_ptr<ITEM, decltype(&free_item)>;

  template <class N, class S>
  default_item(N&& n, S&& s)
    : ptr{nullptr, &free_item},
      name{std::forward<N>(n)},
      desc{std::forward<S>(s)}
    { ptr.reset(new_item(name.data(), desc.data())); }

  item_ptr ptr;
  std::string name;
  std::string desc;
};

template <class T>
struct owning_item : public default_item
{
  using data_type = T;

  data_type user_data;

  template <class N, class S>
  owning_item(N&& n, S&& s)
    : default_item{std::forward<N>(n), std::forward<S>(s)},
      user_data{}
    { ptr->userptr = static_cast<void*>(&data); }
  template <class N, class S, class U>
  owning_item(N&& n, S&& s, U&& u)
    : default_item{std::forward<N>(n), std::forward<S>(s)},
      user_data{std::forward<U>(u)}
    { ptr->userptr = static_cast<void*>(&user_data); }

  data_type* data() { return &user_data; }
};

template <class T>
struct pointer_item : public default_item
{
  using data_type = T;

  template <class N, class S>
  pointer_item(N&& n, S&& s)
    : default_item{std::forward<N>(n), std::forward<S>(s)}
    {}
  template <class N, class S>
  pointer_item(N&& n, S&& s, data_type* u)
    : default_item{std::forward<N>(n), std::forward<S>(s)}
    { ptr->userptr = static_cast<void*>(u); }

  data_type* data() const { return static_cast<data_type*>(ptr->userptr); }
  void data(data_type* n) { ptr->userptr = static_cast<void*>(n); }
};

template <class I = default_item>
class basic_menu
{
  using menu_ptr = std::unique_ptr<MENU, decltype(&free_menu)>;
public:
  using item_type = I;

  enum class opt
  {
    one_value        = O_ONEVALUE,
    show_description = O_SHOWDESC,
    row_major        = O_ROWMAJOR,
    ignore_case      = O_IGNORECASE,
    show_match       = O_SHOWMATCH,
    non_cyclic       = O_NONCYCLIC
  };

  basic_menu();
  ~basic_menu();

  void move_cursor(int dir);
  void post();
  void unpost();
  void add_to_window(basic_window& win);
  void add_entry(std::string&& name, std::string&& description);
  void refresh();
  void set_size(int rows, int cols);
  void set_menu_mark(const char* mark) { ::set_menu_mark(men.get(), mark); }
  bool is_posted() const { return posted; }
  bool is_empty() const { return entries.empty(); }

  void opt_on(opt);
  void opt_off(opt);
  void set_opts(std::initializer_list<opt> opts);
  opt get_opts() const noexcept;

  std::list<item_type>& get_entries()
    { pointers_valid = false; return entries;}
  const std::list<item_type>& get_entries() const { return entries; }
  void set_entries(std::list<item_type>&& e)
    { pointers_valid = false; entries = std::move(e); }

  MENU* get_ptr() { return men.get(); }

private:
  basic_window* win;
  std::list<item_type> entries;
  std::vector<ITEM*> entries_p;
  menu_ptr men;
  bool pointers_valid;
  bool posted;

  int rows = 10;
  int cols = 1;

  void update_entries();
};

typedef basic_menu<default_item> menu;

#include "menu.impl.hpp"

extern template class basic_menu<default_item>;

#endif // SHADOWRUN5_COMBAT_MENU_HPP

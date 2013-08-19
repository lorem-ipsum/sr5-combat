#include "interface.hpp"

#include "ncurses.hpp"
#include <ncurses.h>
#include <algorithm>
#include <clocale>
#include <iomanip>
#include <ios>
#include <memory>
#include <sstream>
#include <string>

namespace
{

std::string
ask_user(const char* question)
{
  mvprintw(LINES - 1, 0, question);
  clrtoeol();
  ncurses::echo(true);
  ncurses::cbreak(false);
  std::string result;
  for (int c = getch(); c != '\n'; c = getch())
    result += c;
  move(LINES - 1, 0);
  clrtoeol();
  ncurses::echo(false);
  ncurses::cbreak(true);
  return result;
}

void
show_error(const char* error)
{
  mvprintw(LINES - 1, 0, error);
  clrtoeol();
}

} // anonymous namespace

interface::interface()
  : win{},
    m{},
    data{}
{
  setlocale(LC_ALL, "");
  ncurses::init();
  ncurses::cbreak(true);
  ncurses::echo(false);
  ncurses::keypad(true);

  win = title_window{LINES - 2,0,0,0,"Testmenü"};
  mvprintw(LINES - 1, 0, "[A]dd");
  win.keypad(true);

  m.set_menu_mark(" ");
  m.set_opts({menu_type::opt::one_value,
              menu_type::opt::show_match});

  m.add_to_window(win);
  m.post();

  win.draw_decoration();
  refresh();
}

void
interface::run()
{
  int c;
  while ((c = wgetch(win.ptr())) != KEY_F(1))
    {
      switch(c)
        {
        case 'a':
          add_char();
          break;
        case KEY_DOWN:
          m.move_cursor(REQ_DOWN_ITEM);
          break;
        case KEY_UP:
          m.move_cursor(REQ_UP_ITEM);
          break;
        case KEY_NPAGE:
          m.move_cursor(REQ_SCR_DPAGE);
          break;
        case KEY_PPAGE:
          m.move_cursor(REQ_SCR_UPAGE);
          break;
        }
    }
}

namespace
{

std::string
gen_display_name(const interface::item_type& item)
{
  std::ostringstream ss;
  ss << std::right << std::setw(2)
     << item.get_data().ini << " "
     << item.get_data().ch->name;
  return ss.str();
}

std::string
gen_marker_name(const interface::item_type& item)
{
  return std::string{"Initiative Pass "} +
    std::to_string(item.get_data().pass);
}

} // anonymous namespace

void
interface::add_char()
{
  auto name = ask_user("Add (Name): ");
  if (!name.empty())
    {
      auto ini = ask_user("Initiative: ");
      character c;
      c.name = std::move(name);
      if (!c.initiative.set_initiative(ini))
        {
          show_error("Invalid initiative value!");
          pos_menu_cursor(m.get_ptr());
          refresh();
          return;
        }
      data.add_character(std::move(c));
      update_entries();
    }
  pos_menu_cursor(m.get_ptr());
  refresh();
}

void
interface::update_entries()
{
  menu_type::entries_type next;
  auto inis = data.get_remaining_turn();
  auto pass = -1;
  for (auto& ini : data.get_remaining_turn())
    {
      if (ini.pass > pass)
        {
          pass = ini.pass;
          next.emplace_back(&gen_marker_name, nullptr,
                            combat::ini{0, pass, nullptr});
          next.back().set_selectable(false);
        }
      next.emplace_back(&gen_display_name, nullptr, std::move(ini));
    }
  m.set_entries(std::move(next));
  m.refresh();
}

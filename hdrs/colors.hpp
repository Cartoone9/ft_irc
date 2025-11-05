#ifndef COLORS_HPP
# define COLORS_HPP

// ─── Base Styles ─────────────────────────────────────────────
# define RESET      "\033[0m"
# define BOLD       "\033[1m"
# define UNDERLINE  "\033[4m"
# define UBOLD		"\033[1;4m"
# define REVERSED   "\033[7m"

// ─── Base Colors (bold + true color) ─────────────────────────
# define RED        "\033[1;38;2;233;0;36m"
# define ORANGE     "\033[1;38;2;255;129;0m"
# define YELLOW     "\033[1;38;2;255;200;50m"
# define GREEN      "\033[1;38;2;75;205;82m"
# define TEAL       "\033[1;38;2;0;180;210m"
# define BLUE       "\033[1;38;2;0;117;226m"
# define PURPLE     "\033[1;38;2;108;86;249m"
# define MAGENTA    "\033[1;38;2;181;51;251m"
# define VIOLET     "\033[1;38;2;204;102;255m"
# define BLACK      "\033[1;38;2;0;0;0m"
# define WHITE      "\033[1;38;2;255;255;255m"

// ─── Underlined Colors (underline + bold + RGB) ──────────────
# define URED       "\033[1;4;38;2;233;0;36m"
# define UORANGE    "\033[1;4;38;2;255;129;0m"
# define UYELLOW    "\033[1;4;38;2;255;200;50m"
# define UGREEN     "\033[1;4;38;2;75;205;82m"
# define UTEAL      "\033[1;4;38;2;0;180;210m"
# define UBLUE      "\033[1;4;38;2;0;117;226m"
# define UPURPLE    "\033[1;4;38;2;108;86;249m"
# define UMAGENTA   "\033[1;4;38;2;181;51;251m"
# define UVIOLET    "\033[1;4;38;2;204;102;255m"
# define UBLACK     "\033[1;4;38;2;0;0;0m"
# define UWHITE     "\033[1;4;38;2;255;255;255m"

#endif // COLORS_HPP

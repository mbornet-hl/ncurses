/* ============================================================================
 * Copyright (C) 2023-2024, Martial Bornet
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *   @(#)  [MB] fd_rectangle.c Version 1.12 du 24/07/22 - 
 *
 *   This is a program to test ncurses before integration into RPN.
 */

// User interface description {{{

/*   - Movements with arrow keys :
 *        right     : one element right
 *        left      : one element left
 *        down      : one line down
 *        up        : one line up
 *
 *   - Other movements :
 *        page down : one screen down
 *        page up   : one screen up
 *        tab       : one screen right
 *        shift tab : one screen left
 *
 *        ^         : go to the beginning of the current line
 *        $         : go to the end of the current line
 *
 *        num |     : goto column num
 *        num _     : goto line num
 *        num G     : goto line num
 *
 *   - Mark a position :
 *        m letter (letter goes from 'a' to 'z')
 *
 *   - Go back to a marked position :
 *        ' letter (letter is a previously marked position)
 *
 *   - Go back to the previous position :
 *        ''
 */

// }}}

// Includes {{{
#include <stdlib.h>
#include <unistd.h>
#include <ncurses.h>
#include <math.h>

// }}}
// Macros definitions {{{
/* Uninitialized coordinate
   ~~~~~~~~~~~~~~~~~~~~~~~~ */
#define   FD_UNDEF_POS   (0)

/* Editor commands
   ~~~~~~~~~~~~~~~ */
#define   FD_CMD_NIL     (0x00)
#define   FD_CMD_MARK    ('m')
#define   FD_CMD_GOTO    ('\'')
#define   FD_CMD_MIDDLE  ('M')

/* Color definitions
   ~~~~~~~~~~~~~~~~~ */
#define   FD_BLUE        (1)
#define   FD_CYAN        (2)
#define   FD_GREEN       (3)
#define   FD_YELLOW      (4)
#define   FD_RED         (5)
#define   FD_RED_REV     (6)
#define   FD_WHITE       (7)

/* Control characters
   ~~~~~~~~~~~~~~~~~~ */
#define   FD_CTRL_B      (0x02)
#define   FD_CTRL_D      (0x04)
#define   FD_CTRL_F      (0x06)
#define   FD_CTRL_U      (0x15)

#define   FD_IS_LOWER(letter) (('a' <= (letter)) && ((letter) <= 'z'))
#define   FD_POS_IDX(letter)  (letter - 'a' + 1)

// }}}
// Structures definitions {{{
struct fd_position {
     int                  i;       // Line number
     int                  j;       // Column number
};
typedef struct fd_position          fd_pos;
typedef struct fd_position         *fd_ref_pos;

struct fd_matrix_elt {
     fd_pos               pos;     // Position
     int                  n;       // Matrix lines number
     int                  p;       // Matrix columns number
};
typedef struct fd_matrix_elt        fd_matrix_elt;
typedef struct fd_matrix_elt       *fd_ref_matrix_elt;

struct fd_sub_matrix {
     int                  dy;      // Height (in lines)
     int                  dx;      // Width (in columns)
};
typedef struct fd_sub_matrix        fd_sub_matrix;
typedef struct fd_sub_matrix       *fd_ref_sub_matrix;

struct fd_rectangle {
     int                  y1;      // Top line number
     int                  x1;      // Leftmost column number
     int                  y2;      // Bottom line number
     int                  x2;      // Rightmost column number
};
typedef struct fd_rectangle         fd_rectangle;
typedef struct fd_rectangle        *fd_ref_rectangle;

// }}}

// fd_draw_rectangle() {{{
/******************************************************************************

                              FD_DRAW_RECTANGLE

******************************************************************************/
void fd_draw_rectangle(fd_ref_rectangle rect)
{
     int                  _x1, _x2, _y1, _y2;

     _x1                 = rect->x1;
     _x2                 = rect->x2;
     _y1                 = rect->y1;
     _y2                 = rect->y2;

     mvhline(_y1, _x1, 0, _x2-_x1);
     mvhline(_y2, _x1, 0, _x2-_x1);
     mvvline(_y1, _x1, 0, _y2-_y1);
     mvvline(_y1, _x2, 0, _y2-_y1);
     mvaddch(_y1, _x1, ACS_ULCORNER);
     mvaddch(_y2, _x1, ACS_LLCORNER);
     mvaddch(_y1, _x2, ACS_URCORNER);
     mvaddch(_y2, _x2, ACS_LRCORNER);
}

// }}}
// fd_value() {{{
/******************************************************************************

                              FD_VALUE

     Generate component value of a fictitious matrix.

******************************************************************************/
int fd_value(fd_ref_matrix_elt matrix_elt)
{
     int                  _i, _j, _n, _p, _val;
     int                  _coeff = 10;

     _i                  = matrix_elt->pos.i;
     _j                  = matrix_elt->pos.j;
     _n                  = matrix_elt->n;
     _p                  = matrix_elt->p;

     if (_i == _j) {
          _val           = _n * _coeff;
     }
     else {
          _val           = (_n - abs(_i - _j)) * _coeff;
     }

     return _val;
}

// }}}
// fd_max() {{{
/******************************************************************************

                         FD_MAX

******************************************************************************/
int fd_max(int x, int y)
{
     int             _max;

     if (x >= y) {
          _max           = x;
     }
     else {
          _max           = y;
     }

     return _max;
}

// }}}
// fd_save_pos() {{{
/******************************************************************************

                         FD_SAVE_POS

******************************************************************************/
void fd_save_pos(fd_ref_matrix_elt ref_mat, fd_ref_pos ref_pos)
{
     ref_pos->i          = ref_mat->pos.i;
     ref_pos->j          = ref_mat->pos.j;
}

// }}}
// fd_copy_pos() {{{
/******************************************************************************

                         FD_COPY_POS

******************************************************************************/
void fd_copy_pos(fd_ref_pos dst_pos, fd_ref_pos src_pos)
{
     dst_pos->i          = src_pos->i;
     dst_pos->j          = src_pos->j;
}

// }}}
// fd_new_pos() {{{
/******************************************************************************

                         FD_NEW_POS

******************************************************************************/
void fd_new_pos(fd_ref_pos curr_pos, fd_ref_pos new_pos, fd_ref_pos old_pos)
{
     if ((curr_pos->i != new_pos->i) || (curr_pos->j != new_pos->j)) {
          /* Store current position
             ~~~~~~~~~~~~~~~~~~~~~~ */
          old_pos->i          = curr_pos->i;
          old_pos->j          = curr_pos->j;

          /* Setup new position
             ~~~~~~~~~~~~~~~~~~ */
          curr_pos->i         = new_pos->i;
          curr_pos->j         = new_pos->j;
     }
}

// }}}
// fd_print_matrix() {{{
/******************************************************************************

                         FD_PRINT_MATRIX

******************************************************************************/
void fd_print_matrix(fd_ref_matrix_elt matrix_elt, fd_ref_sub_matrix delta,
                     fd_ref_rectangle rect, int sz)
{
     char            _buf[256], *_dyn_buf, *_ptr;
     int             _min_width, _value, _color_pair,
                     _x1, _x2, _y1, _y2, _x, _y,
                     _i, _j, _i0, _j0,
                     _n, _p, _dx, _dy;
     fd_matrix_elt   _matrix_elt;

     /* Copy rectangle parameters locally
        ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
     _x1            = rect->x1;
     _x2            = rect->x2;
     _y1            = rect->y1;
     _y2            = rect->y2;

     /* Copy delta parameters
        ~~~~~~~~~~~~~~~~~~~~~ */
     _dx            = delta->dx;
     _dy            = delta->dy;

     /* Copy matrix parameters locally
        ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
     _i0            = matrix_elt->pos.i;
     _j0            = matrix_elt->pos.j;
     _n             = matrix_elt->n;
     _p             = matrix_elt->p;

     _min_width     = _x2 - _x1 + 1;

     /* Check buffer size
        ~~~~~~~~~~~~~~~~~ */
     if (sizeof(_buf) < (_min_width + 1)) {
          if ((_dyn_buf = malloc(_min_width + 1)) == NULL) {
               fprintf(stderr, "Malloc error !\n");
               exit(1);
          }
          _ptr           = _dyn_buf;
     }
     else {
          _dyn_buf       = 0;
          _ptr           = _buf;
     }

     start_color();

     init_color(COLOR_BLACK,    0,   0,     0);
     init_color(COLOR_RED,    800,   0,     0);
     init_color(COLOR_GREEN,    0, 800,     0);
     init_color(COLOR_BLUE,     0, 500,  1000);
     init_color(COLOR_YELLOW, 800, 800,     0);
     init_color(COLOR_CYAN,     0, 1000, 1000);
     init_color(COLOR_WHITE, 1000, 1000, 1000);

     init_pair(FD_BLUE,    COLOR_BLUE,   COLOR_BLACK);
     init_pair(FD_CYAN,    COLOR_CYAN,   COLOR_BLACK);
     init_pair(FD_GREEN,   COLOR_GREEN,  COLOR_BLACK);
     init_pair(FD_YELLOW,  COLOR_YELLOW, COLOR_BLACK);
     init_pair(FD_RED,     COLOR_RED,    COLOR_BLACK);
     init_pair(FD_RED_REV, COLOR_BLACK,  COLOR_RED);
     init_pair(FD_WHITE,   COLOR_WHITE,  COLOR_BLACK);

     /* Initialize next cursor position
        ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
     _y             = _y1 + 1;
     _x             = _x1 + 2;
     move(_y, _x);

     for (_i = _i0; _i < _i0 +_dy; _i++) {
          for (_j = _j0; _j < _j0 + _dx; _j++) {
               int                 _sz = sz + 1;

               _color_pair         = FD_BLUE;
               attron(COLOR_PAIR(_color_pair));
               printw("[");
               _sz--;
               attroff(COLOR_PAIR(_color_pair));

               if (_i == 1) {
                    _color_pair         = FD_GREEN;
               }
               else if (_i == _n) {
                    _color_pair         = FD_RED;
               }
               else if (_i == _j) {
                    _color_pair         = FD_CYAN;
               }
               else {
                    _color_pair         = FD_BLUE;
               }
               attron(COLOR_PAIR(_color_pair));
               printw("%d", _i);
               _sz                 -= sprintf(_buf, "%d", _i);
               attroff(COLOR_PAIR(_color_pair));

               _color_pair         = FD_BLUE;
               attron(COLOR_PAIR(_color_pair));
               printw(", ");
               _sz                 -= 2;
               attroff(COLOR_PAIR(_color_pair));

               if (_j == 1) {
                    _color_pair         = FD_GREEN;
               }
               else if (_j == _p) {
                    _color_pair         = FD_RED;
               }
               else if (_i == _j) {
                    _color_pair         = FD_CYAN;
               }
               else {
                    _color_pair         = FD_BLUE;
               }
               attron(COLOR_PAIR(_color_pair));
               printw("%d", _j);
               _sz                 -= sprintf(_buf, "%d", _j);
               attroff(COLOR_PAIR(_color_pair));

               _color_pair         = FD_BLUE;
               attron(COLOR_PAIR(_color_pair));
               printw("%-*s", _sz, "]");
               attroff(COLOR_PAIR(_color_pair));
          }
          _y++;
          move(_y, _x);

          _matrix_elt.pos.i   = matrix_elt->pos.i;
          _matrix_elt.pos.j   = matrix_elt->pos.j;
          _matrix_elt.n       = matrix_elt->n;
          _matrix_elt.p       = matrix_elt->p;

          for (_j = _j0; _j < _j0 + _dx; _j++) {
               _matrix_elt.pos.i   = _i;
               _matrix_elt.pos.j   = _j;
               _value              = fd_value(&_matrix_elt);
               if (_i > _matrix_elt.n || _j > _matrix_elt.p) {
                    _color_pair         = FD_WHITE;
               }
               else if (_value >= 9000) {
                    _color_pair         = FD_GREEN;
               }
               else if (_value >= 8000) {
                    _color_pair         = FD_YELLOW;
               }
               else if (_value >= 7000) {
                    _color_pair         = FD_RED;
               }
               else if (_value < 0) {
                    _color_pair         = FD_RED_REV;
               }
               attron(COLOR_PAIR(_color_pair));
               printw("%*f", sz, (double) fd_value(&_matrix_elt));
               attroff(COLOR_PAIR(_color_pair));
               printw(" ");
          }
          _y        += 2;
          move(_y, _x);
// mvprintw(y1 + 1, _x1 + 2, s);

     }

     refresh();

     if (_dyn_buf) {
          free(_dyn_buf);
     }
}

// }}}
// fd_get_pos() {{{

/******************************************************************************

                         FD_GET_POS

******************************************************************************/
void fd_get_pos(char letter, fd_pos *pos, fd_matrix_elt *mat, fd_pos *corner)
{
     pos->i         = corner->i + (letter - 'a') / 6;
     pos->j         = corner->j + ((letter - 'a') % 6)
                                   * (8 + log10(mat->n) + log10(mat->p));
}

// }}}
// fd_disp_markers() {{{

/******************************************************************************

                         FD_DISP_MARKERS

******************************************************************************/
void fd_disp_markers(fd_pos *pos, fd_matrix_elt *matrix, fd_pos *corner)
{
     char                 _l;
     int                  _n, _sz_n, _sz_p;
     fd_pos               _pos;

     _sz_n               = log10(matrix->n) + 1;
     _sz_p               = log10(matrix->p) + 1;

     for (_l = 'a'; _l <= 'z'; _l++) {
          _n                  = FD_POS_IDX(_l);
          fd_get_pos(_l, &_pos, matrix, corner);
          move(_pos.i, _pos.j);
          if (pos[_n].i != FD_UNDEF_POS && pos[_n].j != FD_UNDEF_POS) {
               printw("%c[%*d, %*d]", _l, _sz_n, pos[_n].i, _sz_p, pos[_n].j);
          }
     }

     fd_get_pos('z' + 1, &_pos, matrix, corner);
     move(_pos.i, _pos.j);
     _n                  = 0;
     if (pos[_n].i != FD_UNDEF_POS && pos[_n].j != FD_UNDEF_POS) {
          printw("%c[%*d, %*d]", '\'', _sz_n, pos[_n].i, _sz_p, pos[_n].j);
     }
}

// }}}
// fd_init_pos() {{{
/******************************************************************************

                              FD_INIT_POS

******************************************************************************/
void fd_init_pos(fd_ref_pos ref_pos)
{
     int             _i = 0;
     fd_ref_pos      _pos = ref_pos;

     for (_i = 0; _i <= FD_POS_IDX('z'); _i++) {
          _pos->i        = FD_UNDEF_POS;
          _pos->j        = FD_UNDEF_POS;
          _pos++;
     }
}

// }}}
// main() {{{
/******************************************************************************

                              MAIN

******************************************************************************/
int main(int argc, char *argv[])
{
     int                  _ch, _sz, _n = 0, _prev_cmd = FD_CMD_NIL, _i, _j;
     fd_rectangle         _rect;
     fd_matrix_elt        _matrix_elt;
     fd_sub_matrix        _sub_matrix;
     char                 _buf[256];         // XXX
     fd_pos               _pos['z' - 'a' + 2], _prev_pos, _new_pos, _corner;

     /* Check command arguments
        ~~~~~~~~~~~~~~~~~~~~~~~ */
     if (argc != 9) {
          fprintf(stderr, "Usage: %s l c n p i0 j0\n", argv[0]);
          fprintf(stderr, "  l  : line number of rectangle top\n");
          fprintf(stderr, "  c  : column number of rectangle top\n");
          fprintf(stderr, "  n  : number of lines of the matrix\n");
          fprintf(stderr, "  p  : number of columns of the matrix\n");
          fprintf(stderr, "  dy : height of the sub-matrix\n");
          fprintf(stderr, "  dx : width of the sub-matrix\n");
          fprintf(stderr, "  i0 : index of the first sub-matrix element\n");
          fprintf(stderr, "  j0 : index of the first sub-matrix element\n");
          exit(1);
     }

     /* Copy matrix dimensions
        ~~~~~~~~~~~~~~~~~~~~~~ */
     _matrix_elt.n       = atoi(argv[3]);
     _matrix_elt.p       = atoi(argv[4]);
     _matrix_elt.pos.i   = atoi(argv[7]);
     _matrix_elt.pos.j   = atoi(argv[8]);

     /* Copy sub-matrix parameters
        ~~~~~~~~~~~~~~~~~~~~~~~~~~ */
     _sub_matrix.dy      = atoi(argv[5]);
     _sub_matrix.dx      = atoi(argv[6]);

     _sz                 = 12;
     _sz                 = fd_max(_sz, sprintf(_buf, "(%d, %d)", _matrix_elt.n, _matrix_elt.p));

     /* Copy rectangle parameters
        ~~~~~~~~~~~~~~~~~~~~~~~~~ */
     _rect.y1            = atoi(argv[1]);
     _rect.x1            = atoi(argv[2]);
     _rect.y2            = _rect.y1 + (3 * _sub_matrix.dy);
     _rect.x2            = _rect.x1 + ((_sz + 1) * _sub_matrix.dx) + 2;

     /* Initialize position marks
        ~~~~~~~~~~~~~~~~~~~~~~~~~ */
     fd_init_pos(_pos);
     fd_save_pos(&_matrix_elt, &_prev_pos);

     _corner.i           = 1;
     _corner.j           = 60;

     /* Initialize window
        ~~~~~~~~~~~~~~~~~ */
     initscr();               /* Start curses mode */
     raw();                   /* Line buffering disabled */
     keypad(stdscr, TRUE);    /* We get F1, F2 etc.. */
     noecho();                /* Don't echo() while we do getch */

     printw("RPN test of matrix display\n");
     printw("Matrix dimensions        : %5d x %5d\n", _matrix_elt.n, _matrix_elt.p);
     printw("Sub-matrix dimensions    : %5d x %5d\n", _sub_matrix.dy, _sub_matrix.dx);
     printw("Rectangle position       : %3d (lines), %3d (columns)\n", _rect.y1, _rect.x1);
     printw("Rectangle dimensions     : %3d (lines), %3d (columns)\n", _rect.y2, _rect.x2);
     printw("First sub-matrix element : (%d, %d)\n", _matrix_elt.pos.i, _matrix_elt.pos.j);

     /* Draw the borders of the rectangle
        ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
     fd_draw_rectangle(&_rect);

     for (;;) {
          /* Print visible values of the matrix
             ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
          fd_print_matrix(&_matrix_elt, &_sub_matrix, &_rect, _sz);
          refresh();
          move(_rect.y2 + 5, 0);
          _ch            = getch();

          /* Copy coordinates to local variables
             ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
          _i             = _matrix_elt.pos.i;
          _j             = _matrix_elt.pos.j;
          fd_copy_pos(&_new_pos, &_matrix_elt.pos);

          /* Save current position
             ~~~~~~~~~~~~~~~~~~~~~ */
          fd_save_pos(&_matrix_elt, &_prev_pos);

          if (FD_IS_LOWER(_ch)) {
               switch (_prev_cmd) {

               case FD_CMD_MARK:
                    fd_save_pos(&_matrix_elt, &_pos[FD_POS_IDX(_ch)]);
                    _prev_cmd      = FD_CMD_NIL;
                    fd_disp_markers(_pos, &_matrix_elt, &_corner);
                    continue;
                    break;

               case FD_CMD_GOTO:
                    if ((_pos[FD_POS_IDX(_ch)].i != FD_UNDEF_POS)
                    &&  (_pos[FD_POS_IDX(_ch)].j != FD_UNDEF_POS)) {
                         fd_copy_pos(&_new_pos, &_pos[FD_POS_IDX(_ch)]);
                         fd_new_pos(&_matrix_elt.pos, &_new_pos, &_pos[0]); 
                         fd_disp_markers(_pos, &_matrix_elt, &_corner);
                    }
                    _prev_cmd      = FD_CMD_NIL;
                    continue;
                    break;

               default:
                    break;
               }
          }

          switch (_ch) {

          case KEY_LEFT:
               if (_j > 1) {
                    _j--;
               }
               else {
                    // Impossible d'aller a gauche
               }
               break;

          case KEY_RIGHT:
               if ((_j +_sub_matrix.dx - 1) < _matrix_elt.p) {
                    _j++;
               }
               else {
                    // Impossible d'aller a droite
               }
               break;

          case KEY_UP:
               if (_i > 1) {
                    _i--;
               }
               else {
                    // Impossible de remonter
               }
               break;

          case KEY_DOWN:
               if ((_i + _sub_matrix.dy - 1) < _matrix_elt.n) {
                    _i++;
               }
               else {
                    // Impossible de descendre
               }
               break;

          case FD_CTRL_B:
          case KEY_PPAGE:
               if ((_i - _sub_matrix.dy - 1) > 1) {
                    _i        -= _sub_matrix.dy;
               }
               else {
                    _i         = 1;
               }
               break;

          case FD_CTRL_U:
               if ((_i - (_sub_matrix.dy / 2) - 1) > 1) {
                    _i        -= _sub_matrix.dy / 2;
               }
               else {
                    _i         = 1;
               }
               break;

          case FD_CTRL_F:
          case KEY_NPAGE:
               if ((_i + (2 * _sub_matrix.dy) - 1) < _matrix_elt.n) {
                    _i        += _sub_matrix.dy;
               }
               else {
                    _i         = _matrix_elt.n - _sub_matrix.dy + 1;
               }
               break;

          case FD_CTRL_D:
               if ((_i + (3 * _sub_matrix.dy / 2) - 1) < _matrix_elt.n) {
                    _i        += _sub_matrix.dy / 2;
               }
               else {
                    _i         = _matrix_elt.n - _sub_matrix.dy + 1;
               }
               break;

          case FD_CMD_MIDDLE:
               _i        = (_matrix_elt.n - _sub_matrix.dy) / 2 + 1;
               _j        = (_matrix_elt.p - _sub_matrix.dx) / 2 + 1;
               break;

          case KEY_BTAB:
               if ((_j - _sub_matrix.dx - 1) > 1) {
                    _j        -= _sub_matrix.dx;
               }
               else {
                    _j         = 1;
               }
               break;

          case KEY_STAB:
          case '\t':
               if ((_j + (2 * _sub_matrix.dx) - 1) < _matrix_elt.p) {
                    _j        += _sub_matrix.dx;
               }
               else {
                    _j         = _matrix_elt.p - _sub_matrix.dx + 1;
               }
               break;

          case KEY_HOME:
               _i        = 1;
               _j        = 1;
               break;

          case KEY_END:
               _i         = _matrix_elt.n - _sub_matrix.dy + 1;
               break;

          case '^':
               _j        = 1;
               break;

          case '$':
               _j         = _matrix_elt.p - _sub_matrix.dx + 1;
               break;

          case '\033':
          case 'q':
               goto end;
               break;

          case KEY_F(4):
               /* Exit key
                  ~~~~~~~~ */
               printw("F4 Key pressed");
               getch();
               goto end;
               break;

          case '0':
          case '1':
          case '2':
          case '3':
          case '4':
          case '5':
          case '6':
          case '7':
          case '8':
          case '9':
               _n                  = (_n * 10) + (_ch - '0');
               break;

          case KEY_BACKSPACE:
               _n                  /= 10;
               break;

          case 'G':
               if (_n == 0) {
                    _n                  = _matrix_elt.n;
               }
          case '_':
               if (_n == 0) {
                    _n                  = 1;
               }

               if ((_n + _sub_matrix.dy - 1) < _matrix_elt.n) {
                    _i        = _n;
               }
               else {
                    _i         = _matrix_elt.n - _sub_matrix.dy + 1;
               }
               _n                  = 0;
               break;

          case '|':
               if (_n == 0) {
                    _n                  = 1;
               }
               if ((_n +_sub_matrix.dx - 1) < _matrix_elt.p) {
                    _j        = _n;
               }
               else {
                    _j         = _matrix_elt.p - _sub_matrix.dx + 1;
               }
               _n                  = 0;
               break;

          case FD_CMD_MARK:
               _prev_cmd      = FD_CMD_MARK;
               break;

          case FD_CMD_GOTO:
               if (_prev_cmd == FD_CMD_GOTO) {
                    _i             = _pos[0].i;
                    _j             = _pos[0].j;
                    _prev_cmd      = FD_CMD_NIL;
               }
               else {
                    _prev_cmd      = FD_CMD_GOTO;
               }
               break;

          default:
               move(_rect.y2 + 5, 0);
               printw("The pressed key is ");
               attron(A_BOLD);
               printw("0x%02X  ", _ch);
               attroff(A_BOLD);
               break;
          }

          _new_pos.i     = _i;
          _new_pos.j     = _j;

          fd_new_pos(&_matrix_elt.pos, &_new_pos, &_pos[0]);
          fd_disp_markers(_pos, &_matrix_elt, &_corner);
     }

end:
     /* End curses mode
        ~~~~~~~~~~~~~~~ */
     endwin();

     return 0;
}

// }}}

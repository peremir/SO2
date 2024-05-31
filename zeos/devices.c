#include <io.h>
#include <utils.h>
#include <list.h>
#include <devices.h>

// Queue for blocked processes in I/O 
struct list_head blocked;
struct list_head readblocked;

#define MAX_PARAMS 3

const char command_starter = '\[';
const char command_argument_separator = ';';

Byte bg_color;
Byte fg_color;
Byte blink;

Byte default_bg_color = BLACK;
Byte default_fg_color = GREEN;
Byte default_blink = 0;



int sys_write_console(char *buffer, int size) {
    int i,          // Pointer to the character being processed
        param_ptr, // Pointer to the beginning of the parameter
        init_i,     // Pointer to the beginning of the command
        chars_written = 0, // Number of characters written to the screen
        params[MAX_PARAMS],
        n_params;

    bg_color = default_bg_color;
    fg_color = default_fg_color;
    blink = default_blink;

    for (i=0; i<size; i++) {
        if (buffer[i] == command_starter) {
            // A command has started
            init_i = param_ptr = ++i;
            n_params = 0;
            while (i < size) {
                if (is_number(buffer[i])) {
                    ++i;
                }
                else if (buffer[i] == command_argument_separator) {
                  //es un separador de comandos
                    if (i > param_ptr && n_params < MAX_PARAMS) {
                        params[n_params] = atoi_n(&buffer[param_ptr], i-param_ptr);
                        n_params++;
                        param_ptr = i+1;
                    }
                    ++i;
                }
                else if (is_letter(buffer[i])) {
                    // We have a complete command
                    // Parse the last element if needed to
                    if (i > param_ptr && n_params < MAX_PARAMS) {
                        params[n_params] = atoi_n(&buffer[param_ptr], i-param_ptr);
                        n_params++;
                    }
                    // (Try to) execute the command
                    if (execute_command(buffer[i], params, n_params) < 0) {
                        for (; init_i <= i; ++init_i)
                            printc_color(buffer[init_i], fg_color, bg_color, blink);
                        chars_written += i - init_i + 1;
                    }
                    break;
                }
                else {
                    // Invalid character
                    for (; init_i <= i; ++init_i)
                        printc_color(buffer[init_i], fg_color, bg_color, blink);
                    chars_written += i - init_i + 1;
                    break;
                }
            }
        }
        else {
            printc_color(buffer[i], fg_color, bg_color, blink);
            chars_written++;
        }
    }
    return chars_written;
}


int execute_command (char op, int *params, int n_params) {
  switch (op) {
  // Erase character
  case 'K':
      delete_current_char();
      break;
  
  // Change cursor position
  case 'H':
      set_cursor(0, 0);
      break;

  case 'f':
      if (n_params < 2) {
          // Insuficient valid arguments
          return -1;
      }
      set_cursor(params[0], params[1]);
      break;

  // Change screen attributes
  case 'm':
      if (n_params < 1 || n_params > 3) return -1;
      for (int i = 0; i < n_params; ++i)
          set_attribute(params[i]);
      break;
  
  // Command not implemented
  default:
    return -1;
  }

  // Command was executed successfully
  return 0;
}



void set_attribute(int a) {
    switch (a) {
        // Attributes
        case 0: // Reset attributes
            fg_color = default_fg_color;
            bg_color = default_fg_color;
            blink = default_blink;
            break;
        case 1: // Reverse
            short temp = fg_color;
            fg_color = bg_color;
            bg_color = temp;
            break;
        case 5: //blink
            blink = !blink;
            break;

        // Background colors
        case 30: // BLACK
            fg_color = BLACK;
            break;
        case 31: // RED
            fg_color = RED;
            break;
        case 32: // GREEN
            fg_color = GREEN;
            break;
        case 33: // YELLOW
            fg_color = BROWN;
            break;
        case 34: // BLUE
            fg_color = BLUE;
            break;
        case 35: // MAGENTA
            MAGENTA;
            break;
        case 36: // CYAN
            fg_color = CYAN;
            break;
        case 37: // WHITE
            fg_color = LIGHT_GRAY;
            break;

        // Foreground colors
        case 40: // BLACK
            bg_color = BLACK;
            break;
        case 41: // RED
            bg_color = RED;
            break;
        case 42: // GREEN
            bg_color = GREEN;
            break;
        case 43: // YELLOW
            bg_color = BROWN;
            break;
        case 44: // BLUE
            bg_color = BLUE;
            break;
        case 45: // MAGENTA
            bg_color = MAGENTA;
            break;
        case 46: // CYAN
            bg_color = CYAN;
            break;
        case 47: // WHITE
            bg_color = WHITE;
            break;
    }

}

int atoi_n(char *args, int n) {
    int i, num = 0;
    for (i = 0; i < n; ++i) {
        if (is_number(args[i]))
            num = num*10 + (args[i] - '0');
    }
    return num;
}

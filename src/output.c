#include "common.h"
#include "output.h"
#include "process.h"

#define BUFFER_SIZE 256

FILE *fout = NULL;
int cur_style = STYLE_NORMAL;
char buffer[BUFFER_SIZE] = "";

int output_item_nr = 0;

int output_format = 0;

void output_add_buffer(char *text) {
   if ((strlen(buffer) + strlen(text)) >= BUFFER_SIZE) {
      // Text doesn't fit, output current buffer
      if (fout != NULL) {
         fprintf(fout, "%s", buffer);
         strcpy(buffer, "");
      }
   }
   // make sure text fits in buffer. If text it too long to fit in
   // the buffer at all, the buffer has already been output, so just
   // output the text
   if (strlen(text) >= BUFFER_SIZE) {
      if (fout != NULL) {
         fprintf(fout, "%s", text);
      }
      return;
   }
   strcat(buffer, text);
}

/*
 * Determine if the character at the given offset is the first character
 * (of the line, or of a word). This function will check for first
 * character by skipping backwards over any tags.
 */
bool output_char_is_first(char *buffer, int offset) {
   char prev_ch = '\0';
   int j;

   if (offset > 0) {
      j = offset - 1;
      while ((j >= 0) && (buffer[j] == '>')) {
         while ((j >= 0) && (buffer[j] != '<')) {
            j--;
         }
         if ((j >= 0) && (buffer[j] == '<')) {
            j--;
         }
      }
      if (j >= 0) {
         prev_ch = buffer[j];
      }
   }

   return ((prev_ch == '\0') || (prev_ch == ' '));
}

bool output_char_is_last(char *buffer, int offset) {
   char next_ch;
   int j;

   j = offset + 1;
   while (buffer[j] == '<') {
      while ((buffer[j] != '\0') && (buffer[j] != '>')) {
         j++;
      }
      if (buffer[j] == '>') {
         j++;
      }
   }
   next_ch = buffer[j];
   return ((next_ch == '\0')
           || (next_ch == ' ')
           || (next_ch == '"')
           || (next_ch == '\'')
           || (next_ch == '.')
           || (next_ch == ',')
           || (next_ch == '?')
           || (next_ch == '!')
           || (next_ch == '-')
           || (next_ch == ';')
           || (next_ch == ':'));
}

int output_get_next_pos(int pos) {
   int next = pos + 1;
   while (buffer[next] == '<') {
      while (buffer[next] && (buffer[next] != '>')) {
         next++;
      }
      if (buffer[next] == '>') {
         next++;
      }
   }
   return next;
}

int output_get_prev_pos(int pos) {
   int prev = pos;
   if (prev > 0) {
      prev--;
      while ((prev > 0) && (buffer[prev] == '>')) {
         while ((prev > 0) && (buffer[prev] != '<')) {
            prev--;
         }
         if (prev > 0) {
            prev--;
         }
      }
   }
   return prev;
}

void output_flush_buffer() {
   int i;
   bool debug = false;
   if (cur_style & STYLE_ITALIC) {
      output_add_buffer("</i>");
      cur_style = STYLE_NORMAL;
   }
   if (debug) {
      printf("\noutput_flush_buffer(%s):\n", buffer);
   }
   int len = strlen(buffer);
   for (i = 0; i < len; i++) {
      if ((buffer[i] == '\'') && (buffer[i + 1] == '\'')) {
         // change two single quotes to a double quote
         buffer[i] = '"';
         int j;
         for (j = i + 1; j < len; j++) {
            buffer[j] =  buffer[j + 1];
         }
         len--;
      } else if ((buffer[i] == '\\')
                 && (buffer[i + 1] == ':')
                 && (buffer[i + 2] == '2')) {
         // '\:2' indicates the top part of the ':', and should be
         // preceded by a period
         if ((i > 0) && (buffer[i - 1] == '.')) {
            buffer[i - 1] = ':';
            int j;
            for (j = i + 3; j <= len; j++) {
               buffer[j - 3] = buffer[j];
            }
            len -= 3;
         } else {
            buffer[i] = ':';
            int j;
            for (j = i + 3; j <= len; j++) {
               buffer[j - 2] = buffer[j];
            }
            len -= 2;
         }
      } else if ((buffer[i] == '1')
                 && (buffer[i + 1] == ' ')) {
         bool delete_space = false;
         if ((buffer[i + 2] == '\0')
             || (buffer[i + 2] == '.')
             || (buffer[i + 2] == ',')
             || (buffer[i + 2] == ')')
             || ((buffer[i + 2] >= '0')
                 && (buffer[i + 2] <= '9'))
             || ((buffer[i + 2] == 's')
                 && (buffer[i + 3] == 't')
                 && ((buffer[i + 4] == ' ')
                     || (buffer[i + 4] == '.')
                     || (buffer[i + 4] == ',')
                     || (buffer[i + 4] == '-')
                     || (buffer[i + 4] == ':')
                     || (buffer[i + 4] == ';')
                     || (buffer[i + 4] == '?')
                     || (buffer[i + 4] == '!')
                     || (buffer[i + 4] == '\0')))) {
            delete_space = true;
         }
         if (delete_space) {
            int j;
            for (j = i + 1; j < len; j++) {
               buffer[j] =  buffer[j + 1];
            }
            len--;
         }
      } else if ((buffer[i] == '\\')
                 && (buffer[i + 1] == 'I')) {
         int nr_skip = 0;
         // ambiguous 'I' or 'l' -- figure out from the context

         int next_ch = output_get_next_pos(i + 1);
         int next_next_ch = output_get_next_pos(next_ch);
         int prev_ch = output_get_prev_pos(i);
         int prev_prev_ch = output_get_prev_pos(prev_ch);
         int prev_prev_prev_ch = output_get_prev_pos(prev_prev_ch);
         if (debug) {
            printf("buffer='%s'\n", buffer);
            printf("next=%d, next_next=%d, prev=%d, prev_prev=%d, prev_prev_prev=%d\n", next_ch, next_next_ch, prev_ch, prev_prev_ch, prev_prev_prev_ch);
         }
         if ((buffer[next_ch] >= 'A') && (buffer[next_ch] <= 'Z')) {
            if (debug) {
               printf("inside condition 1!\n");
            }
            // if next char is a capital letter, this should be a capital letter
            // too
            buffer[i] = 'I';
            nr_skip = 1;
         } else if (output_char_is_first(buffer, i)
                    && output_char_is_last(buffer, i + 1)) {
            if (debug) {
               printf("inside condition 2!\n");
            }
            // fix stand-alone 'l' to 'I'
            buffer[i] = 'I';
            nr_skip = 1;
         } else if ((buffer[next_ch] == '\\')
                    && (buffer[next_next_ch] == 'I')) {
            if (debug) {
               printf("inside condition 3!\n");
            }
            // double "II" should probably be "ll", unless in a Roman
            // numeral - in which case the "II" is preceded by a "V", "X",
            // "C", "D", "M" or space
            char letter = 'l';
            if ((i > 0) 
                && (buffer[prev_ch] != ' ')
                && (buffer[prev_ch] != 'V')
                && (buffer[prev_ch] != 'X')
                && (buffer[prev_ch] != 'L')
                && (buffer[prev_ch] != 'C')
                && (buffer[prev_ch] != 'D')
                && (buffer[prev_ch] != 'M')) {
               letter = 'l';
            } else {
               letter = 'I';
               buffer[i + 1] = 'I';
            }
            buffer[i] = letter;
            // handle the possible situation that italics start between
            // the current and next character. Remember that when we started
            // in this branch, buffer[i] == '\\' and  buffer[i + 1] == 'I',
            // so buffer[i+2] is the position right after the first ambiguous
            // I/l
            while (buffer[i + 2] == '<') {
               while (buffer[i + 2] && (buffer[i + 2] != '>')) {
                  buffer[i + 1] = buffer[i + 2];
                  i++;
               }
               if (buffer[i + 2] == '>') {
                  i++;
               }
            }
            // At this point, we copied any italics one position back. 
            // Buffer[i+1] is currently undefined and must become the 
            // letter we determined the ambiguous I/l to be. Right now,
            // buffer[i+2] and buffer[i+3] are the second "\\I".
            buffer[i + 1] = letter;
            nr_skip = 2;
         } else if ((i > 0)
                    && (buffer[prev_ch] >= 'A')
                    && (buffer[prev_ch] <= 'Z')
                    && (buffer[next_ch] >= 'a')
                    && (buffer[next_ch] <= 'z')) {
            if (debug) {
               printf("inside condition 4!\n");
            }
            // Capital I followed by a lowercase letter; should be lowercase
            // 'l' if also preceded by a capital letter
            buffer[i] = 'l';
            nr_skip = 1;
         } else if ((prev_ch > 0)
                    && (buffer[prev_ch] == ' ')
                    && ((buffer[prev_prev_ch] == 'I')
                        || (buffer[prev_prev_ch] == 'a')
                        || (buffer[prev_prev_ch] == 'A'))
                    && ((prev_prev_ch == 0)
                        || (buffer[prev_prev_prev_ch] == '-')
                        || (buffer[prev_prev_prev_ch] == ' ')
                        // or if there prev_prev_ch is the first character
                        // but it is italic, prev_prev_prev_ch will be the
                        // start of the italic tag
                        || (buffer[prev_prev_prev_ch] == '<'))) {
            if (debug) {
               printf("inside condition 5!\n");
            }
            // A word starting with a capital 'I', if following either the
            // single-letter word "I" or "A", should probably be starting with
            // a lowercase 'l' instead.
            buffer[i] = 'l';
            nr_skip = 1;
         } else if ((i == 0)
                    || (
                        // is first character: either prev_ch is 0 or
                        // prev_prev_ch is zero and points is '<', so there
                        // really is no prev_prev_ch
                        ((prev_ch == 0) 
                         || ((prev_prev_ch == 0)
                             && (buffer[prev_prev_ch] == '<')))
                        && ((buffer[prev_ch] == '"')
                            || (buffer[prev_ch] == '\'')
                            || (buffer[prev_ch] == '-')))) {
            if (debug) {
               printf("inside condition 6!\n");
            }
            // Start of line - default to capital I
            buffer[i] = 'I';
            nr_skip = 1;
         } else {
            // if prececing letter was a capital, this is a captial too
            if (debug) {
               printf("inside condition 7!\n");
            }
            int j;
            bool in_tag = false;
            for (j = i - 1; j >= 0; j--) {
               if ((buffer[j] >= 'A') && (buffer[j] <= 'Z') && !in_tag) {
                  buffer[i] = 'I';
                  break;
               } else if ((buffer[j] >= 'a') && (buffer[j] <= 'z') && !in_tag) {
                  buffer[i] = 'l';
                  break;
               } else if ((buffer[j] == '.')
                          || (buffer[j] == '?')
                          || (buffer[j] == '!')) {
                  buffer[i] = 'I';
                  break;
               } else if (buffer[j] == '>') {
                  in_tag = true;
               } else if (buffer[j] == '<') {
                  in_tag = false;
               }
            }
            if (j < 0) {
               buffer[i] = 'l';
            }
            nr_skip = 1;
         }
         int j;
         for (j = i + nr_skip; j < len; j++) {
            buffer[j] =  buffer[j + nr_skip];
         }
         len -= nr_skip;;
      } else if (((i + 3) < len)
                 && (buffer[i] == '<')
                 && (buffer[i + 1] == 'i')
                 && (buffer[i + 2] == '>')
                 && (buffer[i + 3] == ' ')) {
         // fix "<i> " to " <i>"
         buffer[i] = ' ';
         buffer[i + 1] = '<';
         buffer[i + 2] = 'i';
         buffer[i + 3] = '>';
      } else if (((i + 4) < len) 
                 && (buffer[i] == ' ')
                 && (buffer[i + 1] == '<')
                 && (buffer[i + 2] == '/')
                 && (buffer[i + 3] == 'i')
                 && (buffer[i + 4] == '>')) {
         // fix " </i>" to "</i> "
         buffer[i] = '<';
         buffer[i + 1] = '/';
         buffer[i + 2] = 'i';
         buffer[i + 3] = '>';
         buffer[i + 4] = ' ';
      } else if (((i + 6) < len)
                 && (buffer[i] == '<')
                 && (buffer[i + 1] == 'i')
                 && (buffer[i + 2] == '>')
                 && (buffer[i + 3] == '<')
                 && (buffer[i + 4] == '/')
                 && (buffer[i + 5] == 'i')
                 && (buffer[i + 6] == '>')) {
         // remove '<i></i>'
         int j;
         for (j = i; (j + 7) <= len; j++) {
            buffer[j] =  buffer[j + 7];
         }
         len -= 7;
         // we need to continue at position i. Decrease i so that when the
         // for loop increases i, it's back where it was
         if (debug) {
            printf("removed <i></i>, i is %d\n", i);
         }
         i--;
      } else if (((i + 6) < len)
                 && (buffer[i] == '<')
                 && (buffer[i + 1] == '/')
                 && (buffer[i + 2] == 'i')
                 && (buffer[i + 3] == '>')
                 && (buffer[i + 4] == '<')
                 && (buffer[i + 5] == 'i')
                 && (buffer[i + 6] == '>')) {
         // remove '</i><i>'
         int j;
         for (j = i; (j + 7) <= len; j++) {
            buffer[j] =  buffer[j + 7];
         }
         len -= 7;
         // we need to continue at position i. Decrease i so that when the
         // for loop increases i, it's back where it was
         if (debug) {
            printf("removed </i><i>, i is %d\n", i);
         }
         i--;
      }
      if (debug) {
         printf("buffer becomes '%s', i=%d\n", buffer, i);
      }
   }

   // post-process on the static buffer: implement replacements that are
   // specified in the character file
   do_postprocess(buffer);

   if (fout != NULL) {
      if (output_format == FORMAT_SRT) {
         fprintf(fout, "%s", buffer);
      } else if (output_format == FORMAT_ASS) {
         // Need to change italic tags
         int start = 0;
         int len = strlen(buffer);
         int i = 0;
         while ((start + i) < len) {
            if (buffer[start + i] == '<') {
               if (i > 0) {
                  fprintf(fout, "%*.*s", i, i, buffer + start);
               }
               start += i;
               i = 0;
               if ((buffer[start + 1] == 'i')
                   && (buffer[start + 2] == '>')) {
                  fprintf(fout, "{\\i1}");
                  start += 3;
               } else if ((buffer[start + 1] == '/')
                          && (buffer[start + 2] == 'i')
                          && (buffer[start + 3] == '>')) {
                  fprintf(fout, "{\\i0}");
                  start += 4;
               } else {
                  start++;
               }
            } else {
               i++;
            }
         }
         if (i > 0) {
            fprintf(fout, "%*.*s", i, i, buffer + start);
         }
      }
   }      
   strcpy(buffer, "");
}

void output_time(long timestamp) {
   output_flush_buffer();

   int seconds = (int)(timestamp / 1000);
   int miliseconds = (int)(timestamp - (1000 * seconds));
   int minutes = seconds / 60;
   seconds = seconds - (60 * minutes);
   int hours = minutes / 60;
   minutes = minutes - (60 * hours);
   if (output_format == FORMAT_SRT) {
      fprintf(fout, "%d:%02d:%02d.%03d", hours, minutes, seconds, miliseconds);
   } else if (output_format == FORMAT_ASS) {
      // use hundredths of seconds instead of thousandths of seconds
      int deciseconds = (miliseconds / 10);
      fprintf(fout, "%d:%02d:%02d.%02d", hours, minutes, seconds, deciseconds);
   }
}

/* ============================================================ */
/* Public interface                                             */
/* ============================================================ */

void output_open(char *fname, int format, int width, int height) {
   fout = fopen(fname, "wt");
   strcpy(buffer, "");
   output_item_nr = 0;
   output_format = format;

   if (output_format == FORMAT_ASS) {
      fprintf(fout, "[Script Info]\n");
      fprintf(fout, "ScriptType: v4.00+\n");
      fprintf(fout, "PlayResX: %d\n", width);
      fprintf(fout, "PlayResY: %d\n", height);
      fprintf(fout, "\n");
      fprintf(fout, "[V4+ Styles]\n");
      fprintf(fout, "Format: Name, Fontname, Fontsize, PrimaryColour, SecondaryColour, OutlineColour, BackColour, Bold, Italic, Underline, BorderStyle, Outline, Shadow, Alignment, MarginL, MarginR, MarginV, AlphaLevel, Encoding\n");

      int fontsize = height / 20;

      // Note: the alignment property is:
      //  1 - bottom left
      //  2 - bottom center
      //  3 - bottom right
      //  4 - middle left
      //  5 - middle center
      //  6 - middle right
      //  7 - top left
      //  8 - top center
      //  9 - top right
      // For now, only prepare for default (bottom), center and top aligned
      // subtitles
      fprintf(fout, "Style: Default,Arial,%d,&Hbbfcff,&Hbbfcff,&H0,&H0,0,0,0,1,1,0,2,10,10,10,0,0\n", fontsize);
      fprintf(fout, "Style: Center,Arial,%d,&Hbbfcff,&Hbbfcff,&H0,&H0,0,0,0,1,1,0,5,10,10,10,0,0\n", fontsize);
      fprintf(fout, "Style: Top,Arial,%d,&Hbbfcff,&Hbbfcff,&H0,&H0,0,0,0,1,1,0,8,10,10,10,0,0\n", fontsize);
      fprintf(fout, "\n");
      fprintf(fout, "[Events]\n");
      fprintf(fout, "Format: Layer, Start, End, Style, Text\n");
   }
}

void output_close() {
   if (fout != NULL) {
      output_flush_buffer();
      fclose(fout);
      fout = NULL;
   }
   output_item_nr = 0;
}

void output_start_item(long start_time, long end_time, position pos) {
   if (fout != NULL) {
      if (output_format == FORMAT_SRT) {
         output_item_nr++;
         output_flush_buffer();
         fprintf(fout, "%d\n", output_item_nr);
         output_time(start_time);
         fprintf(fout, " --> ");
         output_time(end_time);
         output_newline();
      } else if (output_format == FORMAT_ASS) {
         char *style = "Default";
         if (pos == TOP) {
            style = "Top";
         } else if (pos == CENTER) {
            style = "Center";
         } else if (pos != BOTTOM) {
            printf("WARNING: position requested %d not supported\n", pos);
         }
         fprintf(fout, "Dialogue: 0,");
         output_time(start_time);
         fprintf(fout, ",");
         output_time(end_time);
         fprintf(fout, ",%s,", style);
      }
   }
}

void output_end_item() {
   if (fout != NULL) {
      output_flush_buffer();
      if (output_format == FORMAT_SRT) {
         fprintf(fout, "\n");
         // add extra blank line
         fprintf(fout, "\n");
      } else {
         fprintf(fout, "\n");
      }
   }
}

/*
 * Output of actual displayed subtitle content. This is the result of
 * OCR, and therefore must be buffered and post-processed.
 */
void output_string(char *text, int style) {
   if (strlen(text) == 0) {
      return;
   }
   if (!(cur_style & style)) {
      // new style and current style do not match
      if (cur_style & STYLE_ITALIC) {
         // turn off italic
         output_add_buffer("</i>");
         cur_style = STYLE_NORMAL;
      } else if (style & STYLE_ITALIC) {
         // turn on italic
         output_add_buffer("<i>");
         cur_style = STYLE_ITALIC;
      }
   }
   output_add_buffer(text);
}

void output_newline() {
   if (fout != NULL) {
      output_flush_buffer();
      if (output_format == FORMAT_SRT) {
         fprintf(fout, "\n");
      } else if (output_format == FORMAT_ASS) {
         fprintf(fout, "\\N");
      }
   }
}

bool output_is_italic() {
   return (cur_style & STYLE_ITALIC);
}

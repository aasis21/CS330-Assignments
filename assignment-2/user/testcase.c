/*
      ......
      syscall user space code
*/
static int main()
{
  void *ptr1;
  char *ptr = (char *) expand(8, MAP_WR)

  if(ptr == NULL)
              write("FAILED\n", 7);

  *(ptr + 8192) = 'A';   /*Page fault will occur and handled successfully*/

  ptr1 = (char *) shrink(7, MAP_WR);
  *ptr = 'A';          /*Page fault will occur and handled successfully*/

  *(ptr + 4096) = 'A';   /*Page fault will occur and PF handler should termminate
                   the process (gemOS shell should be back) by printing an error message*/
  exit(0);
}

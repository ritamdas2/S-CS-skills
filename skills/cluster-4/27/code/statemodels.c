#include <stdio.h>
#include <math.h>
#include <string.h>

int main(int argc, char **argv)
{
  typdef enum {			// Set of states 
       S0,
       S1,
       S2
       } state_e;

state_e state = S0;	// Initial State
state_e next_state;
event_e event;
int count = 0;
char input[2];
int score = 0;

while(count < 10)			
{
    count = count + 1;
    event = read_event();
	if (state == S0)
	{
    printf("Game Started!\n");
		next_state == S1();
	}
	else if (state == S1)
	{
    printf("The mole is in hole 1! Press 1 to whack it.\n");
		if (atoi(c[0]) == 1)
		{
            score = score + 10;
			next_state == S1;
		}
    else
    {
      printf("Miss!");
    }
  }
  else if (state == S1)
	{
    printf("The mole is in hole 2! Press 2 to whack it.\n");
		if (atoi(c[0]) == 2)
		{
            score = score + 10;
			next_state == S1;
		}
    else
    {
      printf("Miss!");
    }
  }
  else if (state == S1)
	{
    printf("The mole is in hole 3! Press 3 to whack it.\n");
		if (atoi(c[0]) == 3)
		{
            score = score + 10;
			next_state == S1;
		}
    else
    {
      printf("Miss!");
    }
  }
  else if (state == S1)
	{
    printf("The mole is in hole 4! Press 4 to whack it.\n");
		if (atoi(c[0]) == 4)
		{
            score = score + 10;
			next_state == S1;
		}
    else
    {
      printf("Miss!");
    }
  }
  else if (state == S1)
	{
    printf("The mole is in hole 5! Press 5 to whack it.\n");
		if (atoi(c[0]) == 5)
		{
            score = score + 10;
			next_state == S1;
		}
    else
    {
      printf("Miss!");
    }
  }
  else if (state == S1)
	{
    printf("The mole is in hole 6! Press 1 to whack it.\n");
		if (atoi(c[0]) == 6)
		{
            score = score + 10;
			next_state == S1;
		}
    else
    {
      printf("Miss!");
    }
  }
  else if (state == S1)
	{
    printf("The mole is in hole 7! Press 7 to whack it.\n");
		if (atoi(c[0]) == 7)
		{
            score = score + 10;
			next_state == S1;
		}
    else
    {
      printf("Miss!");
    }
  }
  else if (state == S1)
	{
    printf("The mole is in hole 8! Press 8 to whack it.\n");
		if (atoi(c[0]) == 8)
		{
            score = score + 10;
			next_state == S1;
		}
    else
    {
      printf("Miss!");
    }
  }
  else if (state == S1)
	{
    printf("The mole is in hole 9! Press 9 to whack it.\n");
		if (atoi(c[0]) == 9)
		{
            score = score + 10;
			next_state == S2;
		}
    else
    {
      printf("Miss!");
    }
  }
	state = next_state;
  if (count = 15)
  {
    printf("Your score is %d. \n", score);
  }
}
return 0;
}
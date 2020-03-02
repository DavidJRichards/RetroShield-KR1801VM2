/*
command line example for TM1638.
djrm 22 Dec 2018, based on PDP-11/34 console
*/

#include <TM1638.h>
#include <TM1638QYF.h>

// define a module on data pin 18, clock pin 19 and strobe pin 31
//TM1638QYF tm1638(18, 19, 31);
//TM1638QYF* _tm1638;
extern TM1638QYF tm1638;
extern TM1638QYF* _tm1638;

unsigned char ledbit = 1;
char buf[80];

// global text command buffer used to hold digits from keypad
#define CMDLEN 8        // max no of digits
char cmdline[CMDLEN+1]; // add 1 for NULL terminator;
int cmdindex = 0;       // actual number of digits

// global variables
unsigned long load_address = 0777777; // read by DISAD, set by LAD
unsigned long deposit_data = 0123;    // set by DEP, read by EXAM
unsigned long load_status = 0567;     // set by LSR

// lookup table entry used to find ASCII value from button press
/*typedef*/ struct t_keymap {
  const word buttoncode; // binary button code
  const char character;  // ASCII key value
};

#define MAXSIZE 5
struct stack
{
    unsigned int stk[MAXSIZE];
    int top;
};
typedef struct stack STACK;
STACK s;
 
void stk_push(unsigned int);
unsigned int  stk_pop(void);
void stk_display(void);

// 4x4 keypad single press button values mapped to ASCII characters
#define NUM_KEY_LOOKUP 17
t_keymap const keymap[NUM_KEY_LOOKUP] =
{
  {0x0,  '.'},   // default value when search fails
  {0x1,   'A'},  {0x2,   '7'},  {0x4,   'C'},  {0x8,   'D'},
  {0x10,  'E'},  {0x20,  '4'},  {0x40,  '5'},  {0x80,  '6'},
  {0x100, 'I'},  {0x200, '1'},  {0x400, '2'},  {0x800, '3'},
  {0x1000,'M'},  {0x2000,'0'},  {0x4000,'O'},  {0x8000,'P'},
};

// non numeric buttons have commands associated
#define BTN_DISAD 'A'
#define BTN_EXAM  'C'
#define BTN_DEP   'D'
#define BTN_LAD   'E'
#define BTN_LSR   'I'
#define BTN_CLR   'M'
#define BTN_INIT  'O'
#define BTN_CNTRL 'P'

// search for ACSII equivalent to button press
char lookupkey(word mybutton)
{
  int i;
  for(i = 0; i < NUM_KEY_LOOKUP; i++)
  {
    if(keymap[i].buttoncode == mybutton)
      return keymap[i].character; // key found
  }
  return keymap[0].character;     // default
}      

// add key character to line buffer
int bufferupdate(char mykey)
{
  if (BTN_CLR == mykey)        // clear all key buffer
  {
    cmdindex = 0;
    cmdline[cmdindex] = 0;
  }
  else if (BTN_CNTRL == mykey) // erase last key only
  {
    if(cmdindex > 0)
    {
      cmdindex -= 1;
      cmdline[cmdindex] = 0;
    }
  }
  // add digit 0 to 7 to end of buffer
  else if(isdigit(mykey))
  {
    cmdline[cmdindex] = mykey;
    cmdindex += 1;
    cmdline[cmdindex] = 0;
  }
  else // must be a command key
  {
    return mykey;
  }
  return 0;
}

// right justify command line in display
void displayupdate(char* mycmdline)
{
  int i;
  int j = 0;
  char displaybuffer[CMDLEN+1];  
  for(i = cmdindex; i < CMDLEN; i++)
  {
      displaybuffer[j++] = ' ';
  }
  for(i = 0; i < cmdindex; i++)
  {
      displaybuffer[j++] = mycmdline[i];
  }
  displaybuffer[CMDLEN] = 0;
  _tm1638->setDisplayToString(displaybuffer, 1);   
}

void commandprocess(int mycommand)
{
  switch (mycommand) {
  case BTN_DISAD:
    _tm1638->setDisplayToDecNumber(load_address, 1<<7,false);
    stk_display();
    
    break;
  case BTN_DEP:
    deposit_data = strtol(cmdline, NULL, 8);  
    stk_push(deposit_data); 
    _tm1638->setDisplayToDecNumber(deposit_data, 1<<6, false);
    bufferupdate(BTN_CLR);
    break;
  case BTN_EXAM:
    deposit_data=stk_pop();
    _tm1638->setDisplayToDecNumber(deposit_data, 1<<6, false);
    bufferupdate(BTN_CLR);
    break;
  case BTN_LAD:
//    load_address =stk_pop() + strtol(cmdline, NULL, 8); 
    load_address = stk_pop() + stk_pop(); 
    stk_push(load_address);
    _tm1638->setDisplayToDecNumber(load_address, 1<<7,false);
    bufferupdate(BTN_CLR);
    break;
  case BTN_LSR:
//    load_address = stk_pop() - strtol(cmdline, NULL, 8); 
    load_address = stk_pop() - stk_pop(); 
    stk_push(load_address);
    _tm1638->setDisplayToDecNumber(load_address, 1<<7,false);
    bufferupdate(BTN_CLR);
    break;
  default:
    displayupdate(cmdline);       
    break;
  }
}

//--------------------------------------------

void panel_update(void) {

  static word prevbutton = 0; // used to prevent button autorepeat
  word buttons;               // actual key code of button pressed
  char keypress;              // ASCII translation 
  int command;                // non numeric command key
 
  buttons = _tm1638->getButtons();
  if (buttons != 0) // button pressed 
  {
    if (buttons != prevbutton) // if new key press
    {
        keypress = lookupkey(buttons);  // convert to ASCII
        command=bufferupdate(keypress); // add digit to commandline buffer
        commandprocess(command);        // process command button press
    }
    prevbutton=buttons; // save key press for test above
  }
  else
  {
    if(prevbutton != 0)   // only update prevbutton if neccessary
    {
      prevbutton=buttons; // allow new button when finger off button (0)
    }
  }
}

//--------------------------------------------------------------

/*  Function to add an element to the stack */
void stk_push (unsigned int num)
{
    if (s.top == (MAXSIZE - 1))
    {
        Serial.print("Stack is Full\n");
        return;
    }
    else
    {
        s.top = s.top + 1;
        s.stk[s.top] = num;
        stk_display();
    }
    return;
}

/*  Function to delete an element from the stack */
unsigned int stk_pop ()
{
    unsigned int num;
    if (s.top == (int)- 1)
    {
       Serial.write("Stack is Empty\n");
        return (s.top);
    }
    else
    {
        num = s.stk[s.top];
        stk_display();
        s.top = s.top - 1;
    }
    return(num);
}

/*  Function to display the status of the stack */
void stk_display ()
{
    int i;
    if (s.top == -1)
    {
        Serial.print("Stack is empty\n");
        return;
    }
    else
    {
        Serial.print("\n The status of the stack is \n");
        for (i = s.top; i >= 0; i--)
        {
          sprintf(buf, "Element %d, Value %j\n",i, s.stk[i]);
          Serial.print(buf);
        }
    }
    Serial.print("\n");
}

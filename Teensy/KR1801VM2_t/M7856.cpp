// DEC M7856 DL11-W Serial IO Card 

#include "M7856.H"

// Bit definitions for status registers

#define M7856_RXSTATUS_RCVRACT 11
#define M7856_RXSTATUS_RCVRDONE 7
#define M7856_RXSTATUS_RCVRINTEND 6
#define M7856_RXSTATUS_RDREND 1

#define M7856_TXSTATUS_XMITRDY 7
#define M7856_TXSTATUS_XMITINTENB 6
#define M7856_TXSTATUS_MAINT 2
#define M7856_TXSTATUS_BREAK 1

#define M7856_LTC_INTMON 7
#define M7856_LTC_INTENB 6


M7856::M7856(uint8_t usart,unsigned int baseaddress,unsigned int baud) {
  
  this->rxstatusaddress = baseaddress;
  this->rxdataaddress = baseaddress+2;
  this->txstatusaddress = baseaddress+4;
  this->txdataaddress = baseaddress+6;
  this->ltcaddress = (baseaddress == 0177560) ? baseaddress - 012 : 0;
  this->usart = usart;
  if (baud > 0)
  {
    switch (this->usart) {
      case 0:
        break;
      case 1:
        Serial1.begin(baud);
        Serial1.println("Serial 1 initialised");
        break;
      case 5:
        Serial5.begin(baud);
        Serial5.println("Serial 5 initialised");
        break;
      default:
        break;
    }
  }
  init();
  
}

void M7856::init() {
  this->rxdata = 0;
  this->txdata = 0;
  this->rxstatus = 0;
  this->txstatus = (1 << M7856_TXSTATUS_XMITRDY) ;
}

void M7856::write(unsigned int address, unsigned int value) {
  char tmp[80];
    if (address == this->rxstatusaddress) 
    {
      this->rxstatus = value & ((1<<M7856_RXSTATUS_RCVRINTEND) | (1<<M7856_RXSTATUS_RDREND));         
          sprintf(tmp,  "rx status write %o", value);
          Serial.println(tmp);
    } 
    else if (address == this->txstatusaddress) 
    {
          sprintf(tmp,  "tx status write %o", value);
          Serial.println(tmp);
      this->txstatus = value & ((1<<M7856_TXSTATUS_XMITINTENB) | (1<<M7856_TXSTATUS_MAINT) | (1<<M7856_TXSTATUS_BREAK));         
    } 
    else if (address == this->rxdataaddress) 
    {
          sprintf(tmp,  "rx data");
          Serial.println(tmp);
      ; 
    }
    else if (address == this->txdataaddress) 
    {
      this->txdata = value;
      this->txstatus = this->txstatus & ((1<<M7856_TXSTATUS_XMITINTENB) | (1<<M7856_TXSTATUS_MAINT) | (1<<M7856_TXSTATUS_BREAK));
//    sprintf(tmp,  "data<%x>", this->txdata);
//    Serial.println(tmp);
      switch (this->usart) {
        case 1:
          Serial1.write(this->txdata);
          sprintf(tmp,  "usart 1 tx data write %o", value);
          Serial.println(tmp);
          break;
        case 5: 
          Serial5.write(this->txdata);
          sprintf(tmp,  "usart 5 tx data write %o", this->txdata);
          Serial.println(tmp);
          break;
        default:
          break;
      }
    }
}

unsigned int M7856::read(unsigned int address) {
  char tmp[80];
      switch (this->usart) {
        case 1:
          sprintf(tmp,  "usart 1 read");
          Serial.print(tmp);
          break;
        case 5: 
          sprintf(tmp,  "usart 5 read");
          Serial.print(tmp);
          break;
        default:
          Serial.print("usart ?");
          break;
      }
    if (address == this->rxstatusaddress) 
    {
      sprintf(tmp,  " rx status %o", this->rxstatus);
      Serial.println(tmp);
      return this->rxstatus;      
    } 
    else if (address == this->txstatusaddress) 
    {
      sprintf(tmp,  " tx status %o", this->txstatus);
      Serial.println(tmp);
      return this->txstatus;      
    } 
    else if (address == this->rxdataaddress) 
    {
      Serial.println(this->rxdata);
      this->rxstatus = this->rxstatus & ((1<<M7856_RXSTATUS_RCVRINTEND) | (1<<M7856_RXSTATUS_RDREND));         
      return this->rxdata;      
    } 
    else if (address == this->txdataaddress) 
    {
      Serial.println(" txd");
      return 0;
    }
#warning address error code here?    
      Serial.println("address error");
    return 0;// suppress compiler warning
}

boolean M7856::here(unsigned int address) {
  return((this->rxstatusaddress == address) || (this->rxdataaddress == address) || (this->txdataaddress == address) || (this->txstatusaddress == address) || (this->ltcaddress == address));
}

void M7856::event() {
  char tmp[80];
  switch (this->usart)
  {
    case 1:
      if(Serial1.availableForWrite()) {
        this->txstatus = this->txstatus | (1<<M7856_TXSTATUS_XMITRDY);
      } else {
        this->txstatus = this->txstatus & 0177577;
      }

      if( (Serial1.available()) && !(this->rxstatus & (1<<M7856_RXSTATUS_RCVRDONE))) {
        this->rxdata = Serial1.read();
        sprintf(tmp,  "usart 1 receive data %o ", this->rxdata);
        Serial.println(tmp);
        this->rxstatus = this->rxstatus | (1<<M7856_RXSTATUS_RCVRDONE);
      }
      break;
      
    case 5:
      if( (Serial5.availableForWrite())) {
        this->txstatus = this->txstatus | (1<<M7856_TXSTATUS_XMITRDY);
      } else {
        this->txstatus = this->txstatus & 0177577;
      }

      if( (Serial5.available()) && !(this->rxstatus & (1<<M7856_RXSTATUS_RCVRDONE))) {
    sprintf(tmp,  "usart 5 receive data %o ", this->rxdata);
    Serial.println(tmp);
        this->rxdata = Serial5.read();
        this->rxstatus = this->rxstatus | (1<<M7856_RXSTATUS_RCVRDONE);
      }
      break;
      
    default:
      break;
  }
}

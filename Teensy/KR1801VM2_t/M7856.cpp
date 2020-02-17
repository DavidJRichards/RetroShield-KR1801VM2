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
      case 2:
      Serial2.begin(baud);
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
    if (address == this->rxstatusaddress) {
      this->rxstatus = value & ((1<<M7856_RXSTATUS_RCVRINTEND) | (1<<M7856_RXSTATUS_RDREND));         
    } else if (address == this->txstatusaddress) {
      this->txstatus = value & ((1<<M7856_TXSTATUS_XMITINTENB) | (1<<M7856_TXSTATUS_MAINT) | (1<<M7856_TXSTATUS_BREAK));         
    } else if (address == this->rxdataaddress) {
      
    } else if (address == this->txdataaddress) {
      this->txdata = value;
      this->txstatus = this->txstatus & ((1<<M7856_TXSTATUS_XMITINTENB) | (1<<M7856_TXSTATUS_MAINT) | (1<<M7856_TXSTATUS_BREAK));
//    sprintf(tmp,  "data<%x>", this->txdata);
//    Serial.println(tmp);
      switch (this->usart) {
        case 1:
          Serial1.write(this->txdata);
          break;
        case 2: 
          Serial2.write(this->txdata);
          break;
        default:
          break;
      }
    }
}

unsigned int M7856::read(unsigned int address) {
    if (address == this->rxstatusaddress) {
      return this->rxstatus;      
    } else if (address == this->txstatusaddress) {
      return this->txstatus;      
    } else if (address == this->rxdataaddress) {
      this->rxstatus = this->rxstatus & ((1<<M7856_RXSTATUS_RCVRINTEND) | (1<<M7856_RXSTATUS_RDREND));         
      return this->rxdata;      
    } else if (address == this->txdataaddress) {
      return 0;
    }
#warning address error code here?    
    return 0;// suppress compiler warning
}

boolean M7856::here(unsigned int address) {
  return((this->rxstatusaddress == address) || (this->rxdataaddress == address) || (this->txdataaddress == address) || (this->txstatusaddress == address) || (this->ltcaddress == address));
}

void M7856::event() {
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
        this->rxstatus = this->rxstatus | (1<<M7856_RXSTATUS_RCVRDONE);
      }
      break;
      
    case 2:
      if( (Serial2.availableForWrite())) {
        this->txstatus = this->txstatus | (1<<M7856_TXSTATUS_XMITRDY);
      } else {
        this->txstatus = this->txstatus & 0177577;
      }

      if( (Serial2.available()) && !(this->rxstatus & (1<<M7856_RXSTATUS_RCVRDONE))) {
        this->rxdata = Serial2.read();
        this->rxstatus = this->rxstatus | (1<<M7856_RXSTATUS_RCVRDONE);
      }
      break;
      
    default:
      break;
  }
}

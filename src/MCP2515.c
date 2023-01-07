#include "SPI.h"
#include "MCP2515.h"

/* MCP2515 Initialize */
bool MCP2515_Initialize(void){
  SPI_CS_Disable();    
      
  return false;
}

/* MCP2515 Set Configuration Mode */
bool MCP2515_SetConfigMode(void){

  /* CANCTRL Register Configuration Mode */  
  MCP2515_WriteByte(MCP2515_CANCTRL, MODE_CONFIG);
  
  uint8_t loop = 10;
  
  do{    
    /* Confirm Operation */    
    if((MCP2515_ReadByte(MCP2515_CANSTAT) & MODE_MASK) == MODE_CONFIG){
      return true;
    }
    loop--;
  }while(loop > 0); 
  
  return false;
}

/* MCP2515 Set Normal Mode */
bool MCP2515_SetNormalMode(void){

  /* CANCTRL Register Normal Mode */  
  MCP2515_WriteByte(MCP2515_CANCTRL, MODE_NORMAL);
  
  uint8_t loop = 10;
  
  do{    
    /* Confirm Operation */    
    if((MCP2515_ReadByte(MCP2515_CANSTAT) & MODE_MASK) == MODE_NORMAL){
      return true;
    }
    loop--;
  }while(loop > 0);
  
  return false;
}

/* MCP2515 SPI Reset */
void MCP2515_Reset(void){    
  SPI_CS_Enable();
      
  SPI_WriteByte(MCP2515_RESET);
      
  SPI_CS_Disable();
}

/* Read Byte */
uint8_t MCP2515_ReadByte (uint8_t address){
  uint8_t retVal;
  
  SPI_CS_Enable();
  
  SPI_WriteByte(MCP2515_READ);
  SPI_WriteByte(address);
  retVal = SPI_ReadByte();
      
  SPI_CS_Disable();
  
  return retVal;
}

/* Read Sequential Bytes */
void MCP2515_ReadRxSequence(uint8_t instruction, uint8_t *data, uint8_t length){
  SPI_CS_Enable();
  
  SPI_WriteByte(instruction);        
  SPI_ReadBytes(data, length);
    
  SPI_CS_Disable();
}

/* Write Byte */
void MCP2515_WriteByte(uint8_t address, uint8_t data){    
  SPI_CS_Enable();  
  
  SPI_WriteByte(MCP2515_WRITE);
  SPI_WriteByte(address);
  SPI_WriteByte(data);  
    
  SPI_CS_Disable();
}

/* Write Sequential Bytes */
void MCP2515_WriteByteSequence(uint8_t startAddress, uint8_t endAddress, uint8_t *data){    
  SPI_CS_Enable();
  
  SPI_WriteByte(MCP2515_WRITE);
  SPI_WriteByte(startAddress);
  SPI_WriteBytes(data, (endAddress - startAddress + 1));
  
  SPI_CS_Disable();
}

/* Write Sequential Bytes to TxBuffer */
void MCP2515_LoadTxSequence(uint8_t instruction, uint8_t *idReg, uint8_t dlc, uint8_t *data){    
  SPI_CS_Enable();
  
  SPI_WriteByte(instruction);
  SPI_WriteBytes(idReg, 4);
  SPI_WriteByte(dlc);
  SPI_WriteBytes(data, dlc);
       
  SPI_CS_Disable();
}

/* Write Byte to TxBuffer */
void MCP2515_LoadTxBuffer(uint8_t instruction, uint8_t data){
  SPI_CS_Enable();
  
  SPI_WriteByte(instruction);
  SPI_WriteByte(data);
        
  SPI_CS_Disable();
}

/* Transmit TxBuffer using RTS Command */
void MCP2515_RequestToSend(uint8_t instruction){
  SPI_CS_Enable();
  
  SPI_WriteByte(instruction);
      
  SPI_CS_Disable();
}

/* MCP2515 Read Status */
uint8_t MCP2515_ReadStatus(void){
  uint8_t retVal;
  
  SPI_CS_Enable();
  
  SPI_WriteByte(MCP2515_READ_STATUS);
  retVal = SPI_ReadByte();
        
  SPI_CS_Disable();
  
  return retVal;
}

/* MCP2515 Get RxStatus */
uint8_t MCP2515_GetRxStatus(void){
  uint8_t retVal;
  
  SPI_CS_Enable();
  
  SPI_WriteByte(MCP2515_RX_STATUS);
  retVal = SPI_ReadByte();
        
  SPI_CS_Disable();
  
  return retVal;
}

/* Change Register Value */
void MCP2515_BitModify(uint8_t address, uint8_t mask, uint8_t data){    
  SPI_CS_Enable();
  
  SPI_WriteByte(MCP2515_BIT_MOD);
  SPI_WriteByte(address);
  SPI_WriteByte(mask);
  SPI_WriteByte(data);
        
  SPI_CS_Disable();
}

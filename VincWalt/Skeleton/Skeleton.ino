////////////////////////////
//
// V21.1 Skeleton.ino
//
//
// 2022-12-17 Jens Andersson
//
////////////////////////////

//
// Select library
#include <datacommlib.h>

//
// Prototypes
//
// predefined functions
void l1_send(unsigned long l2frame, int framelen);
boolean l1_receive(int timeout);
// your own

//
// Runtime
//

// Runtime variables

// State
int state = NONE;

//////////////////////////////////////////////////////////
//
// Add global constant and variable declarations here
//
Shield sh;  // note! no () since constructor takes no arguments
Transmit tx;
Receive rx;

//////////////////////////////////////////////////////////

//
// Code
//
void setup() {
  sh.begin();
  //////////////////////////////////////////////////////////
  //
  // Add init code here
  //

  state = APP_PRODUCE;

  // Set your development node's address here

  //////////////////////////////////////////////////////////
}

void loop() {

  //////////////////////////////////////////////////////////
  //
  // State machine
  // Add code for the different states here
  //

  switch (state) {

    case L1_SEND:
      Serial.println("[State] L1_SEND");
      // +++ add code here and to the predefined function void l1_send(unsigned long l2frame, int framelen) below
      // Serial.println(tx.message);
      l1_send(tx.frame, LEN_FRAME);
      state = HALT;
      break;

    case L1_RECEIVE:
      Serial.println("[State] L1_RECEIVE");
      // +++ add code here and to the predefined function boolean l1_receive(int timeout) below
      boolean recOK = l1_receive(20000);
      // ---
      break;

    case L2_DATA_SEND:
      Serial.println("[State] L2_DATA_SEND");

      state = L1_SEND;
      break;

    case L2_RETRANSMIT:
      Serial.println("[State] L2_RETRANSMIT");
      // +++ add code here

      // ---
      break;

    case L2_FRAME_REC:
      Serial.println("[State] L2_FRAME_REC");
      // +++ add code here

      // ---
      break;

    case L2_ACK_SEND:
      Serial.println("[State] L2_ACK_SEND");
      // +++ add code here

      // ---
      break;

    case L2_ACK_REC:
      Serial.println("[State] L2_ACK_REC");
      // +++ add code here

      // ---
      break;

    case APP_PRODUCE:
      Serial.println("[State] APP_PRODUCE");
      tx.message[MESSAGE_PAYLOAD] = sh.select_led();
      Serial.print("MSG PAYLOAD:");
      Serial.println(tx.message[MESSAGE_PAYLOAD]);
      tx.frame_from          = 0;
      tx.frame_to            = 0;
      tx.frame_seqnum        = 0;
      tx.frame_crc           = 0;
      tx.frame_payload       = tx.message[MESSAGE_PAYLOAD];
      tx.frame_type          = FRAME_TYPE_DATA;
      tx.frame_generation();

      state = L1_SEND;

      break;

    case APP_ACT:
      Serial.println("[State] APP_ACT");
      // +++ add code here

      // ---
      break;

    case HALT:
      Serial.println("[State] HALT");
      sh.halt();
      break;

    default:
      Serial.println("UNDEFINED STATE");
      break;
  }

  //////////////////////////////////////////////////////////
}

//////////////////////////////////////////////////////////
//
// Add code to the predefined functions
//

void l1_send(unsigned long frame, int framelen) {
  int bit_to_send;

  for (int i = 0; i < LEN_PREAMBLE; i++) {
    bit_to_send = (PREAMBLE_SEQ << i) & 0x80;
    if (bit_to_send != 0) {
      digitalWrite(PIN_TX, HIGH);
    }
    else {
      digitalWrite(PIN_TX, LOW);
    }
    delay(T_S);
  }
  for (int i = 0; i < LEN_SFD; i++) {
    bit_to_send = (SFD_SEQ << i) & 0x80;
    if (bit_to_send != 0) {
      digitalWrite(PIN_TX, HIGH);
    }
    else {
      digitalWrite(PIN_TX, LOW);
    }
    delay(T_S);
  }
  for (int i = framelen - 1; i >= 0; i--) {
    bit_to_send = (frame >> i) & 0x01;
    if (bit_to_send == 1) {
      digitalWrite(PIN_TX, HIGH);
    }
    else {
      digitalWrite(PIN_TX, LOW);
    }
    delay(T_S);
    Serial.println(frame);
  }
}

boolean l1_receive(int timeout) {
  
  long start_time = millis();
  byte SFD_tester = 0x00;
  
  Serial.println("Looking for preamble...");
  
  while (sh.sampleRecCh(PIN_RX) == 0) {
    if (start_time >= timeout) {
      Serial.println("No preamble detected. Timing out...");
      return false;
    }
  }


  delay(T_S/2);
  
  //Preamble start detected, Looking for SFD
  Serial.println("Preamble start detected, Looking for SFD...");

  while (SFD_tester != SFD_SEQ) {
    SFD_tester = SFD_tester << 1 | sh.sampleRecCh(PIN_RX);
  }
  //Här är SFD_tester == SFD_SEQ
  
  digitalWrite(DEB_1, HIGH);
  unsigned long buf2 = 0x00000000;
  
  for(int i = 0; i < LEN_FRAME; i++){
    byte rx_val = sh.sampleRecCh(PIN_RX);
    buf2 = (buf2 << 1) | rx_val;
    digitalWrite(DEB_2, rx_val);
    Serial.println("rx_value: ");
    Serial.print(rx_val);
  }
  Serial.println(buf2, BIN);
  byte inputFrame = buf2;
  digitalWrite(DEB_2, HIGH);
  return true;
}

//////////////////////////////////////////////////////////
//
// Add your functions here
//

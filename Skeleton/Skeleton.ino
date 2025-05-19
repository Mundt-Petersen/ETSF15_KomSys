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
bool l1_receive(int timeout);
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
int seqnum;
//////////////////////////////////////////////////////////

//
// Code
//
void setup() {

  //////////////////////////////////////////////////////////
  //
  // Add init code here
  //
  sh.begin();
  sh.setMyAddress(5);
  seqnum = 0;
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
      state = L1_RECEIVE;
      break;

    case L1_RECEIVE:
      Serial.println("[State] L1_RECEIVE");
      // +++ add code here and to the predifend function boolean l1_receive(int timeout) below

      if (l1_receive(20000)) {
        state = L2_FRAME_REC;
      } else {
        state = L2_RETRANSMIT;
      }

      // ---
      break;

    case L2_DATA_SEND:
      Serial.println("[State] L2_DATA_SEND");
      Serial.print("MSG PAYLOAD:");
      Serial.println(tx.message[MESSAGE_PAYLOAD]);
      tx.frame_from          = sh.getMyAddress();
      tx.frame_to            = tx.message[MESSAGE_ADDRESS];
      tx.frame_seqnum        = seqnum++;
      tx.frame_crc           = 0;
      tx.frame_payload = tx.message[MESSAGE_PAYLOAD];
      tx.frame_type = FRAME_TYPE_DATA;
      tx.frame_generation();
      tx.add_crc(genCRC());

      tx.print_frame();

      state = L1_SEND;
      break;

    case L2_RETRANSMIT:
      Serial.println("[State] L2_RETRANSMIT");
      if (tx.tx_attempts == 2) {
        Serial.println("Transmission failed");
        state = APP_PRODUCE;
      } else {
        tx.tx_attempts++;
        state = L1_SEND;
      }
      break;

    case L2_FRAME_REC:
      Serial.println("[State] L2_FRAME_REC");
      // +++ add code here
      if (!check_CRC()) {
        state = L1_RECEIVE;
      }
      else {
        rx.frame_decompose();
        rx.print_frame();
        if (sh.getMyAddress() == rx.frame_to && rx.frame_type == FRAME_TYPE_ACK) {
          state = L2_ACK_REC;
        }
        else {
          state = L1_RECEIVE;
        }
      }


      // ---
      break;

    case L2_ACK_SEND:
      Serial.println("[State] L2_ACK_SEND");
      // +++ add code here

      // ---
      break;

    case L2_ACK_REC:
      Serial.println("[State] L2_ACK_REC");
      if (tx.frame_seqnum == rx.frame_seqnum) {
        state = APP_PRODUCE;
      }
      else {
        state = L2_RETRANSMIT;
      }
      break;

    case APP_PRODUCE:
      Serial.println("[State] APP_PRODUCE");
      tx.tx_attempts = 0;
      tx.message[MESSAGE_PAYLOAD] = sh.select_led();
      tx.message[MESSAGE_ADDRESS] = sh.get_address();

      state = L2_DATA_SEND;

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
    //Serial.println(frame);
  }
}

bool l1_receive(int timeout) {

  byte buffer = 0b00000000;
  unsigned long start_time = millis();
  int tracker = 0;
  bool foundSFD = false;
  bool foundPREM = false;
  unsigned long DATA = 0;

  while (!foundPREM) {
    if (millis() - start_time >= timeout) {
      Serial.println("Timing out...");
      return false;
    }
    if (sh.sampleRecCh(PIN_RX) == 1) {
      foundPREM = true;
      Serial.println("Preamble found");
    }
  }
  delay(T_S / 2);

  //start_time = millis();
  while (!foundSFD) {

    buffer = (buffer << 1) | sh.sampleRecCh(PIN_RX);

    //Serial.print("SFD_SEQ = ");
    //Serial.println(SFD_SEQ);
    //Serial.print("Buffer = ");
    //Serial.println(buffer);

    if (buffer == SFD_SEQ) {
      foundSFD = true;
      Serial.println("SFD found");
    } else if (millis() - start_time >= timeout) {
      Serial.println("Timing out...");
      return foundSFD;
    }
    /*
      Serial.print("Start time = ");
      Serial.println(start_time);
      Serial.print("Time now = ");
      Serial.println(now);
    */
    delay(T_S);
  }

  for (int i = 0; i < 32; i++) {
    DATA = (DATA << 1) | sh.sampleRecCh(PIN_RX);
    delay(T_S);



  }
  rx.frame = DATA;
  return true;
}


boolean check_CRC() {
  byte shifterCheck = 0b00000000;
  unsigned long DATA = rx.frame;
  byte newBit;
  byte msb;
  unsigned long G = 0xA7;
  for (int i = 0; i < 32; i++) {
    msb = (shifterCheck >> 7) & 0x01;
    newBit = (DATA >> 31 - i) & 0x01;
    shifterCheck = (shifterCheck << 1) | newBit;
    if (msb == 1) {

      shifterCheck = shifterCheck ^ G;

    }
  }

  Serial.print("shifterCheck ");
  Serial.println(shifterCheck);
  return (shifterCheck == 0);
}

int genCRC() {
  byte shifter = 0b00000000;
  unsigned long DATA = tx.frame;
  byte newBit;
  byte msb;
  unsigned long G = 0xA7;
  for (int i = 0; i < 32; i++) {
    msb = (shifter >> 7) & 0x01;
    newBit = (DATA >> 31 - i) & 0x01;
    shifter = (shifter << 1) | newBit;

    if (msb == 1) {
      shifter = shifter ^ G;
    }
  }
  Serial.print("Shifter = ");
  Serial.println(shifter);

  return shifter;
}

//////////////////////////////////////////////////////////
//
// Add your functions here
//

// V9821s controls HT1621 in PZEM-021 V5.0
//
// Arduino IDE/framework C/C++ code for an AVR microcontroller, a "Digispark" clone (AVR/Microchip ATtiny85 with "micronucleus" USB bootloader)
//
// The ATtiny85 is running at 16.5 MHz
// The bootloader takes a few seconds before the user program is executed!
//

// pins
#define HT_RES  5 // (requires AVR fuse to disable reset to use as IO)
//              4 // USB
//              3 // USB
#define HT_CS   2 //
#define HT_WR   1 //
#define HT_DATA 0 //


// global variables used in "loop"
int old = HIGH, edge = HIGH;
bool s = false;
unsigned long time;

// display memory (only lower nibble is used)
uint8_t ram[32] = {0};
uint8_t buf[32] = {0};


// seven segment display

// naming convention
// (clockwise from top)
//   A
// F   B
//   G
// E   C
//   D

// seven segment bits
// (so a digit/letter can be encoded in a byte)
#define S_A   1
#define S_B   2
#define S_C   4
#define S_D   8
#define S_E  16
#define S_F  32
#define S_G  64
//          128


// seven segment glyphs
// (which segment is active in a digit/letter)

// digits
//      G_x  S_A   S_B   S_C   S_D   S_E   S_F   S_G
#define G_0 (S_A + S_B + S_C + S_D + S_E + S_F +  0 )
#define G_1 ( 0  + S_B + S_C +  0  +  0  +  0  +  0 )
#define G_2 (S_A + S_B +  0  + S_D + S_E +  0  + S_G)
#define G_3 (S_A + S_B + S_C + S_D +  0  +  0  + S_G)
#define G_4 ( 0  + S_B + S_C +  0  +  0  + S_F + S_G)
#define G_5 (S_A +  0  + S_C + S_D +  0  + S_F + S_G)
#define G_6 (S_A +  0  + S_C + S_D + S_E + S_F + S_G)
#define G_7 (S_A + S_B + S_C +  0  +  0  +  0  +  0 )
#define G_8 (S_A + S_B + S_C + S_D + S_E + S_F + S_G)
#define G_9 (S_A + S_B + S_C +  0  +  0  + S_F + S_G)
//      G_x  S_A   S_B   S_C   S_D   S_E   S_F   S_G

// letters
//      G_x  S_A   S_B   S_C   S_D   S_E   S_F   S_G
#define G_A (S_A + S_B + S_C +  0  + S_E + S_F + S_G) // A  0
#define G_B ( 0  +  0  + S_C + S_D + S_E + S_F + S_G) // b  1
#define G_C ( 0  +  0  +  0  + S_D + S_E +  0  + S_G) // c  2
#define G_D ( 0  + S_B + S_C + S_D + S_E +  0  + S_G) // d  3
#define G_E (S_A +  0  +  0  + S_D + S_E + S_F + S_G) // E  4
#define G_F (S_A +  0  +  0  +  0  + S_E + S_F + S_G) // F  5
#define G_G (S_A + S_B + S_C + S_D +  0  + S_F + S_G) // g  6
#define G_H ( 0  +  0  + S_C +  0  + S_E + S_F + S_G) // h  7
#define G_I ( 0  + S_B + S_C +  0  +  0  +  0  +  0 ) // I  8
#define G_J ( 0  + S_B + S_C + S_D +  0  +  0  +  0 ) // J  9
#define G_K ( 0  +  0  +  0  +  0  +  0  +  0  + S_G) // - 10
#define G_L ( 0  +  0  +  0  + S_D + S_E + S_F +  0 ) // L 11
#define G_M ( 0  +  0  +  0  +  0  +  0  +  0  + S_G) // - 12
#define G_N ( 0  +  0  + S_C +  0  + S_E +  0  + S_G) // n 13
#define G_O ( 0  +  0  + S_C + S_D + S_E +  0  + S_G) // o 14
#define G_P (S_A + S_B +  0  +  0  + S_E + S_F + S_G) // P 15
#define G_Q (S_A + S_B + S_C +  0  +  0  + S_F + S_G) // q 16
#define G_R ( 0  +  0  +  0  +  0  + S_E +  0  + S_G) // r 17
#define G_S (S_A +  0  + S_C + S_D +  0  + S_F + S_G) // S 18
#define G_T ( 0  +  0  +  0  + S_D + S_E + S_F + S_G) // t 19
#define G_U ( 0  + S_B + S_C + S_D + S_E + S_F +  0 ) // U 20
#define G_V ( 0  +  0  + S_C + S_D + S_E +  0  +  0 ) // v 21
#define G_W ( 0  +  0  +  0  +  0  +  0  +  0  + S_G) // - 22
#define G_X ( 0  +  0  +  0  +  0  +  0  +  0  + S_G) // - 23
#define G_Y ( 0  + S_B + S_C + S_D +  0  + S_F + S_G) // y 24
#define G_Z (S_A + S_B +  0  + S_D + S_E +  0  + S_G) // Z 25
//      G_x  S_A   S_B   S_C   S_D   S_E   S_F   S_G


// arrays with digits and letters
//
// ASCII offset is '0' or 48 or 0x30
const uint8_t digits[] = {
  G_0, G_1, G_2, G_3, G_4, G_5, G_6, G_7, G_8, G_9,
  };
// ASCII offset is 'A' or 65 or 0x41
const uint8_t letters[] = {
  G_A, G_B, G_C, G_D, G_E, G_F, G_G, G_H, G_I, G_J,
  G_K, G_L, G_M, G_N, G_O, G_P, G_Q, G_R, G_S, G_T,
  G_U, G_V, G_W, G_X, G_Y, G_Z,
};



// bit indexes for LCD segments in LCD controller RAM
//
//   88.88 V    88.88 A
//    voltage    current
//   888.8 kW   8888 kWh
//    power      energy
//
// voltage
//                          A   B   C   D   E   F   G
const uint8_t v[4][7] = {{  3,  7,  5,  0,  1,  2,  6}, //
                         { 11, 15, 13,  8,  9, 10, 14}, //
                         { 19, 23, 21, 16, 17, 18, 22}, //
                         { 27, 31, 29, 24, 25, 26, 30}};//
const uint8_t v_d =  12; // decimal 88.88
const uint8_t v_u =  28; // unit (V)
const uint8_t v_l =   4; // label

// current
const uint8_t c[4][7] = {{ 35, 39, 37, 32, 33, 34, 38}, //
                         { 43, 47, 45, 40, 41, 42, 46}, //
                         { 51, 55, 53, 48, 49, 50, 54}, //
                         { 59, 63, 61, 56, 57, 58, 62}};//
const uint8_t c_d =  44; // decimal 88.88
const uint8_t c_u =  60; // unit (A)
const uint8_t c_l =  36; // label

// power
const uint8_t p[4][7] = {{ 64, 68, 70, 67, 66, 65, 69}, //
                         { 72, 76, 78, 75, 74, 73, 77}, //
                         { 80, 84, 86, 83, 82, 81, 85}, //
                         { 88, 92, 94, 91, 90, 89, 93}};//
const uint8_t p_d =  87; // decimal 888.8
const uint8_t p_k = 111; // kilo
const uint8_t p_u = 103; // unit (W)
const uint8_t p_l =  79; // label

// energy
const uint8_t e[4][7] = {{ 96,100,102, 99, 98, 97,101}, //
                         {104,108,110,107,106,105,109}, //
                         {112,116,118,115,114,113,117}, //
                         {120,124,126,123,122,121,125}};//
const uint8_t e_k = 127; // kilo
const uint8_t e_u = 119; // unit (Wh)
const uint8_t e_l =  95; // label



// set single bit with index <bit> in <ram>
void setbit(uint8_t bit){
  ram[bit/4] |= 1<<(bit%4);
}//setbit

// clear single bit with index <bit> in <ram>
void clrbit(uint8_t bit){
  ram[bit/4] &= ~(1<<(bit%4));
}//clrbit

// clear <ram>
void clear(){
  for (uint8_t i=0; i<32; i++) {
    ram[i] = 0;
  }
}//clear

// write <c> from <glyphs[]> to <digit[]>
void write(uint8_t c, const uint8_t glyphs[], const uint8_t digit[7]){
  for (uint8_t i=0; i<7; i++) {
    if (glyphs[c] & (1<<i)) {
      setbit( digit[i] );
    }
    else {
      clrbit( digit[i] );
    }
  }
}//write


// show all bits/segments in <digit[]> sequentially
void test(const uint8_t digit[7]){
  for (uint8_t i=0; i<7; i++) {
      setbit( digit[i] );
      writedisplay(ram);
      delay(300);
  }
}//test

// show all segments sequentially
void segments(){
  for (uint8_t i=0; i<132; i++) {
      setbit( i );
      writedisplay(ram);
      clrbit( i );
      delay(300);
  }
}//segments


// array to store active segments/bits by index
uint8_t bits[128] = {0};

// get active bits/segments from <in> and store their index in <out>, return the count
uint8_t getbits(uint8_t in[], uint8_t out[]){
  uint8_t n=0;
  for(uint8_t i=0; i<128; i++){
    if ( in[i/4] & (1<<(i%4)) ) {
      out[n++] = i;
    }
  }
  return n;
}//getbits


// bit-bang synchronous serial for HT1621
// write all 32x4 bits from <data> successively
// (this assumes the display has been initialised)
void writedisplay(uint8_t data[]) {

  digitalWrite(HT_CS, LOW);

  // command
  digitalWrite(HT_WR, LOW);
  digitalWrite(HT_DATA, HIGH);  // 1
  digitalWrite(HT_WR, HIGH);

  digitalWrite(HT_WR, LOW);
  digitalWrite(HT_DATA, LOW);   // 0
  digitalWrite(HT_WR, HIGH);

  digitalWrite(HT_WR, LOW);
  digitalWrite(HT_DATA, HIGH);  // 1
  digitalWrite(HT_WR, HIGH);

  // 6 bits start address 0b000000
  digitalWrite(HT_DATA, LOW);
  for(uint8_t n=0; n<6; n++){
    digitalWrite(HT_WR, LOW);
    digitalWrite(HT_WR, HIGH);  // 0
  }

  // 32x4 bits, LSB first
  for(uint8_t a=0; a<32; a++){
    for(uint8_t d=0; d<4; d++){

      // clock
      digitalWrite(HT_WR, LOW);

      // data
      if ( (data[a]>>d)&1  ) {
        digitalWrite(HT_DATA, HIGH);
      }
      else {
        digitalWrite(HT_DATA, LOW);
      }

      // clock
      digitalWrite(HT_WR, HIGH);

    }
  }

  digitalWrite(HT_CS, HIGH);

} // writedisplay


// bit-bang synchronous serial for HT1621
// receive a synchronous serial data packet for HT1621 and store bits in <data>
// probably ONLY WORKS WITH panel meter "PZEM-021 v5.0" (timing, packets, line levels, ..)
void readdisplay(uint8_t data[]) {

  uint8_t address = 0;

  // wait for end of an ongoing "packet" (rising edge)
  while( digitalRead(HT_CS) == LOW  );

  // wait for start of packet (falling edge)
  while( digitalRead(HT_CS) == HIGH  );

  // command
  // wait for rising edge
  while( digitalRead(HT_WR) == HIGH  );
  while( digitalRead(HT_WR) == LOW  );
  if( digitalRead(HT_DATA) != HIGH ) return;  // 1

  while( digitalRead(HT_WR) == HIGH  );
  while( digitalRead(HT_WR) == LOW  );
  if( digitalRead(HT_DATA) != LOW ) return;   // 0

  while( digitalRead(HT_WR) == HIGH  );
  while( digitalRead(HT_WR) == LOW  );
  if( digitalRead(HT_DATA) != HIGH ) return;  // 1

  // address (6 bits, 5 used)
  for(uint8_t i=0; i<6; i++){
    // wait for rising edge
    while( digitalRead(HT_WR) == HIGH  );
    while( digitalRead(HT_WR) == LOW  );

    // read bit
    if( digitalRead(HT_DATA) == HIGH ){
      // MSB first
      address |= 0b100000>>i;
    }
  }

  // data bits (multiples of 4)
  while(1){

    // try to read 4 bits
    for(uint8_t i=0; i<4; i++){

      // wait for falling edge or end of packet
      while( digitalRead(HT_WR) == HIGH && digitalRead(HT_CS) == LOW  );
      // end of packet?
      if( digitalRead(HT_CS) == HIGH ){
        return;
      }
      // wait for rising edge
      while( digitalRead(HT_WR) == LOW  );

      // read data bit
      if( digitalRead(HT_DATA) == HIGH ){
        //set
        data[address] |= 1<<i;
      }
      else {
        //clear
        data[address] &= ~(1<<i);
      }

    }//for

    // increment address for "successive address writing" mode
    address++;

  }//while

}//readdisplay



// "Arduino" framework init function
void setup() {
  // remember this is run _after_ the bootloader has finished listening (about 6 seconds after power-on)
  // nothing to do
}//setup


// "Arduino" framework main loop
void loop() {

  // detect rising edge
  edge = digitalRead(HT_CS);
  if( edge && edge!=old) {
    time = millis();
    s = true;
  }
  old = edge;

  // detect gap between display updates
  if( s && ( (millis() - time) >= 15) ){
    s = false;


    // listen to display data (16 packets x 2x4 bits = 128 bits)
    for(uint8_t n=0; n<16; n++){
      readdisplay(buf);
    }

    // reset V9821S and ...
    pinMode(HT_RES, OUTPUT);
    //digitalWrite(5,LOW);  // redundant
    delay(10);  // see V9821S datasheet

    // ... take control of serial interface
    pinMode(HT_CS,   OUTPUT);
    digitalWrite(HT_CS,HIGH); // prevent later V9821S glitch: it pulls #WR down once on CS falling edge in reset
    pinMode(HT_WR,   OUTPUT);
    pinMode(HT_DATA, OUTPUT);

    // get bit indexes from data
    uint8_t k = getbits(buf,bits);

    // shuffle the bits
    randomSeed(7331);
    for(uint8_t n=0; n<k; n++){
      uint8_t r = random(n,k);
      uint8_t tmp = bits[n];
      bits[n] = bits[r];
      bits[r] = tmp;
    }

    // clear the randomized segments
    uint8_t d = 1000/k;
    for(uint8_t n=0; n<k; n++){
      buf[bits[n]/4] &= ~(1<<(bits[n]%4));
      writedisplay(buf);
      delay(d);
    }

    // show all segments sequentially
    segments();

    // show seven-segments sequentially
    test(v[0]);
    test(v[1]);
    test(v[2]);
    test(v[3]);

    test(c[0]);
    test(c[1]);
    test(c[2]);
    test(c[3]);

    test(p[0]);
    test(p[1]);
    test(p[2]);
    test(p[3]);

    test(e[0]);
    test(e[1]);
    test(e[2]);
    test(e[3]);



    // show all digits
    for (uint8_t d=0; d<10; d++) {
      clear();

      write(d, digits, v[0]);
      write(d, digits, v[1]);
      write(d, digits, v[2]);
      write(d, digits, v[3]);

      write(d, digits, c[0]);
      write(d, digits, c[1]);
      write(d, digits, c[2]);
      write(d, digits, c[3]);

      write(d, digits, p[0]);
      write(d, digits, p[1]);
      write(d, digits, p[2]);
      write(d, digits, p[3]);

      write(d, digits, e[0]);
      write(d, digits, e[1]);
      write(d, digits, e[2]);
      write(d, digits, e[3]);

      writedisplay(ram);
      delay(400);
    }

    // "type" some text
    #define TYPEDELAY 300
    clear();
    writedisplay(ram);

    write('P'-'A', letters, v[0]);
    writedisplay(ram); delay(TYPEDELAY);
    write('Z'-'A', letters, v[1]);
    writedisplay(ram); delay(TYPEDELAY);
    write('E'-'A', letters, v[2]);
    writedisplay(ram); delay(TYPEDELAY);
    write('M'-'A', letters, v[3]);
    writedisplay(ram); delay(TYPEDELAY);

    write('X'-'A', letters, c[0]);
    writedisplay(ram); delay(TYPEDELAY);
    write(0, digits, c[1]);
    writedisplay(ram); delay(TYPEDELAY);
    write(2, digits, c[2]);
    writedisplay(ram); delay(TYPEDELAY);
    write(1, digits, c[3]);
    writedisplay(ram); delay(TYPEDELAY);

    // -
    writedisplay(ram); delay(TYPEDELAY);
    write('V'-'A', letters, p[1]);
    writedisplay(ram); delay(TYPEDELAY);
    write(5, digits, p[2]);
    writedisplay(ram); delay(TYPEDELAY);
    setbit(p_d);
    writedisplay(ram); delay(TYPEDELAY);
    write(0, digits, p[3]);
    writedisplay(ram); delay(TYPEDELAY);

    write('L'-'A', letters, e[0]);
    writedisplay(ram); delay(TYPEDELAY);
    write('C'-'A', letters, e[1]);
    writedisplay(ram); delay(TYPEDELAY);
    write('D'-'A', letters, e[2]);
    writedisplay(ram); delay(TYPEDELAY);
    // write('O'-'A', letters, e[3]);
    // writedisplay(ram); delay(TYPEDELAY);


    // release serial interface
    pinMode(HT_CS,   INPUT);
    pinMode(HT_WR,   INPUT);
    pinMode(HT_DATA, INPUT);

    // release V9821S
    pinMode(HT_RES,  INPUT);

    // STOP
    while(1);

  } // gap detected

}//loop


// EOF

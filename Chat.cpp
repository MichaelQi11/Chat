#include <Arduino.h>

int serverPin = 13;

void setup() {
  init();
  pinMode(serverPin,INPUT);
  Serial.begin(9600);
  Serial3.begin(9600);
}

// set a function to calculate two 31 bit number within 32 bit
uint32_t fasttimeMod(uint32_t a, uint32_t b,uint32_t m) {
// let a and b mod m first
// set the variable result
  uint32_t newa = a % m;
  uint32_t bi = b % m;
  uint32_t result = 0;

  // result = b0*2^0 + b1*2^1 + ... + bi*2^i
  for(int i = 0; i < 32; i++) {
    uint32_t prea = newa;
    newa = newa >> 1;
    if(prea != newa << 1) {
      result = (result + bi) % m;
    }
    bi = (2 * bi) % m;
  }
  return result;
}

// set a function for calculating created keys
uint32_t powMod(uint32_t a, uint32_t b, uint32_t m) {
  // compute ap[0] = a
  // ap[1] = ap[0]*ap[0]
  // ...
  // ap[i] = ap[i-1]*ap[i-1] (all mod m) for i >= 1

	uint32_t result = 1 % m;
	uint32_t sqrVal = a % m;
	uint32_t newB = b;

  // result = a^{binary number represented the first i bits of b}
  // sqrVal = a^{2^i}
  // newB = b >> i
	while (newB > 0) {
		if (newB & 1) {
			result = fasttimeMod(result,sqrVal,m);
		}
		sqrVal = fasttimeMod(sqrVal,sqrVal,m);
		newB = (newB >> 1);
	}
	return result;
}

// set a function to test if the calculated key is correct
void onePowModFastTest(uint32_t a, uint32_t b, uint32_t m, uint32_t expected){
  uint32_t calculated = powMod(a, b, m);
  // print error and expected value if the calculated key is not correct
  if (calculated != expected){
    Serial.println("error in powMod test");
    Serial.print("expected:");
    Serial.println(expected);
    Serial.print("got:");
    Serial.println(calculated);
  }
}

void testPowModFast() {
	Serial.println("starting tests");
	// run powMod through a series of tests
	onePowModFastTest(1, 1, 20, 1); //1^1 mod 20 == 1
	onePowModFastTest(5, 7, 37, 18);
	onePowModFastTest(5, 27, 37, 31);
	onePowModFastTest(3, 0, 18, 1);
	onePowModFastTest(2, 5, 13, 6);
	onePowModFastTest(1, 0, 1, 0);
	onePowModFastTest(123456, 123, 17, 8);

	Serial.println("Now starting big test");
	uint32_t start = micros();
	onePowModFastTest(123, 2000000010ul, 17, 16);
	uint32_t end = micros();
	Serial.print("Microseconds for big test: ");
	Serial.println(end-start);

	Serial.println("Another big test");
	onePowModFastTest(123456789, 123456789, 61234, 51817);

	Serial.println("With a big modulus (< 2^31)");
	onePowModFastTest(123456789, 123456789, 2147483647, 1720786551);


	Serial.println("tests done!");
}

// set a function to create a random private key
uint16_t randomPrivateKey(){
  // create a variable to store the key
	uint16_t PrivateKey;
	int i;
  // use analog pin to generate random integer
  for (i = 0; i < 16; i++){
		uint16_t num = analogRead(A1) & 1;
		PrivateKey = PrivateKey | (num << i);
	}
	return PrivateKey;
}

// set a function to use the function above
// create keys and share to each arduino
uint32_t diffiHellman(){
	const uint32_t p = 2147483647;
	const uint32_t g = 16807;
  // create a private key first
  uint32_t privateKey = randomPrivateKey();
	// create a public key prepare to share to other arduino
	uint32_t publicKey = powMod(g, privateKey, p);
	return publicKey;
}

uint32_t next_key(uint32_t current_key) {
  const uint32_t modulus = 0x7FFFFFFF; // 2^31-1
  const uint32_t consta = 48271;  // we use that consta<2^16
  uint32_t lo = consta*(current_key & 0xFFFF);
  uint32_t hi = consta*(current_key >> 16);
  lo += (hi & 0x7FFF)<<16;
  lo += hi>>15;
  if (lo > modulus) lo -= modulus;
  return lo;
}

// the function used to send message
void sender(uint32_t secretKey) {
  uint32_t currentkey = secretKey;
  // when there is letter available to send
  while(Serial.available() > 0) {
  // read the typed bytes
  int byteSend = Serial.read();
  // encypt the byte and send it to the other arduino
  uint8_t encryptedChar= byteSend ^ currentkey;
  Serial3.write(encryptedChar);
  Serial.write(byteSend);

  // when enter is pressed, stop sending letter and send enter to other arduino
    if(byteSend == 13) {
      Serial.write("\n");
      uint8_t encryptedLineFeed= 10 ^ currentkey;
      Serial3.write(encryptedLineFeed);
      break;
    }
    currentkey = next_key(currentkey);
  }
}

// use this function to receive the message
void receiver(uint32_t secretKey) {
  uint32_t currentkey = secretKey;
  // wait until the sender has sent some message
  while(Serial3.available() > 0) {
    // read the message sent from the other arduino
    uint8_t byteRead = Serial3.read();
    // decrypt the message and write the decrpted message
    uint8_t decryptedChar = byteRead ^ currentkey;
    Serial.write(decryptedChar);
    currentkey = next_key(currentkey);
  }
}

// Waits for a certain number of bytes on Serial3 or timeout
// nbytes: the number of bytes we want
// timeout: timeout period (ms); specifying a negative number
// turns off timeouts (the function waits indefinitely if timeouts are turned off)
// return True if the required number of bytes have arrived
bool wait_on_serial3(uint8_t nbytes, long timeout) {
  unsigned long deadline = millis() + timeout;
  while(Serial3.available() < nbytes && (timeout < 0 || millis() < deadline))
  {
    delay(1);
  }
  return Serial3.available() >= nbytes;
}

// Write an uint32_t to Serial3, starting from the least-significant and
// finishing with the most significant byte.
void uint32_to_serial3(uint32_t num) {
  Serial3.write((char)(num >> 0));
  Serial3.write((char)(num >> 8));
  Serial3.write((char)(num >> 16));
  Serial3.write((char)(num >> 24));
}

// Reads an uint32_t from Serial3, starting from the least-significant and
// finishing with the most significant byte.
uint32_t uint32_from_serial3() {
  uint32_t num = 0;
  num = num | ((uint32_t) Serial3.read()) << 0;
  num = num | ((uint32_t) Serial3.read()) << 8;
  num = num | ((uint32_t) Serial3.read()) << 16;
  num = num | ((uint32_t) Serial3.read()) << 24;
  return num;
}

// set a function to represent server
uint32_t Server() {
  // generate a random skey and set the variable ckey
  uint32_t skey = diffiHellman();
  uint32_t ckey = 0;;
  // if the server receive CR and ckey, store ckey and send ACK and skey
  while(true) {
    if(wait_on_serial3(5,1000)) {
      if(Serial3.read() == 'C') {
        ckey = uint32_from_serial3();
        Serial3.write('A');
        uint32_to_serial3(skey);
        break;
      }
    }
  }
  // if the server receives ACK, calculate ckey^skey mod 2147483647
  // if the server receives CR, store ckey and keep reading
  while(true) {
    if(wait_on_serial3(1,1000)) {
      char byteRead = Serial3.read();
      if(byteRead == 'C')
      {
        ckey = uint32_from_serial3();
      }
      else if(byteRead == 'A') {
        break;
      }
    }
  }
  return powMod(ckey, skey, 2147483647);
}

// set a function for client
uint32_t client() {
  // generate a random ckey
  uint32_t ckey = diffiHellman();
  // if not receive ACK and skey, keep sending CR and ckey
  do {
    Serial3.write('C');
    uint32_to_serial3(ckey);
  }
  while(!wait_on_serial3(5,1000));
  // read skey and store it
  Serial3.read();
  uint32_t skey = uint32_from_serial3();
  Serial3.write('A');
  return powMod(ckey, skey, 2147483647);
}

int main() {
  setup();
  testPowModFast();

  // read from server pin to set the value of server
  boolean server = digitalRead(serverPin);
  Serial.print("you are: ");
  if(server) {
    Serial.println("server");
  }
  else {
    Serial.println("client");
  }

  // set the variable of secretkey and get it from server or client regarding on
  // user's state
  uint32_t secretKey = 0;
  if(server) {
    secretKey = Server();
  }
  else {
    secretKey = client();
  }
  Serial.println("The shared key is:");
  Serial.println(secretKey);

  // start chatting
  while(true){
    sender(secretKey);
    receiver(secretKey);
  }

  // set arduino to wait until the message
  // have printed on the screen
  Serial3.flush();
  Serial.flush();
  return 0;
}

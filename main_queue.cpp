#include "mbed.h"
#include "FXOS8700.h"
#include "Hexi_KW40Z.h"
#include "Hexi_OLED_SSD1351.h"
#include "OLED_types.h"
#include "FXAS21002.h"
#include "OpenSans_Font.h"
#include "string.h"

#define LED_ON      0
#define LED_OFF     1
#define M_PI  3.14159265

typedef struct {
  int cmd;
  float move_x;
  float move_y;
} message_t;


void StartHaptic(void);
void StopHaptic(void const *n);
void dataTask(void);

MemoryPool<message_t, 16> mpool;
Queue<message_t, 16> queue;



DigitalOut redLed(LED1,1);
DigitalOut greenLed(LED2,1);
DigitalOut blueLed(LED3,1);
DigitalOut haptic(PTB9);
Serial pc(USBTX, USBRX); // Serial interface


/* Define timer for haptic feedback */
RtosTimer hapticTimer(StopHaptic, osTimerOnce);

/* Instantiate the Hexi KW40Z Driver (UART TX, UART RX) */
KW40Z kw40z_device(PTE24, PTE25);
FXOS8700 accel(PTC11, PTC10);
FXAS21002 gyro(PTC11,PTC10); // Gyroscope

/* Instantiate the SSD1351 OLED Driver */
SSD1351 oled(PTB22,PTB21,PTC13,PTB20,PTE6, PTD15); /* (MOSI,SCLK,POWER,CS,RST,DC) */

/*Create a Thread to handle sending BLE Sensor Data */
//Thread txThread;
Thread dataThread;
Timer t; 

 /* Text Buffer */
char text[20];
int16_t instruction = 0;
bool flip;
bool click = false;
/****************************Call Back Functions*******************************/


void ButtonRight(void)
{
    StartHaptic();
    kw40z_device.ToggleAdvertisementMode();
}

void ButtonLeft(void)
{
    StartHaptic();
    kw40z_device.ToggleAdvertisementMode();
}

void ButtonUp(void)
{
    StartHaptic();
    //redLed = !redLed;
    click = true;
}

void PassKey(void)
{
    StartHaptic();
    strcpy((char *) text,"PAIR CODE");
    oled.TextBox((uint8_t *)text,0,25,95,18);

    /* Display Bond Pass Key in a 95px by 18px textbox at x=0,y=40 */
    sprintf(text,"%d", kw40z_device.GetPassKey());
    oled.TextBox((uint8_t *)text,0,40,95,18);
}

const uint8_t *image1; // Pointer for the image1 to be displayed
char text1[20]; // Text Buffer for dynamic value displayed
char text2[20]; // Text Buffer for dynamic value displayed
char text3[20];
/////ML coffeee
/////ML coff


float my_dot(float a[6],float b[6]) {
    float result = 0;
    for(int i=0;i<6;i++){
        result = result + a[i]*b[i];
        }
    return result;
    }

void get_command(int& cmd,float& move_x,float& move_y,int sample_length){

  const float coffe_0[6] = { 3.26058485e-03, -4.89987206e-03, -2.05877017e-02,  3.16671988e-03, 5.79273382e+00, -1.60204263e+00};

  const float coffe_1[6] = { 1.22703505e-03, -2.06662456e-04, -1.68982080e-02,  1.94460189e-02, -4.50079549e+00,  1.09530008e+00};

  const float coffe_2[6] = {-2.84085412e-04, -3.18166798e-03, -7.44055441e-03,  2.84450999e-02, -1.19638051e+00,  5.25903292e+00};

  const float coffe_3[6] = { 7.09153276e-03, -4.21382224e-04, -1.51337127e-02,  5.92974483e-03, 2.22769372e+00, -8.17349130e+00};

  const float bias_0 = -2.069;
  const float bias_1 = -2.005;
  const float bias_2 = -1.976;
  const float bias_3 = -2.617;
  float ax_modify = 0;
  float ay_modify = 0;
  float dt = 0.2;
  float roll = 0;
  float pitch = 0;
  float alpha = 0.6;
  float roll_acc = 0;
  float pitch_acc = 0;
  float acc_g_x = 0; ///acceleration due to gravity
  float acc_g_y = 0;
  float accel_data[3]; // Storage for the data from the sensor
  float ax, ay;//
  float az = 10; // Integer value from the sensor to be displayed
  float gyro_data[3];  // Storage for the data from the sensor
  float gx, gy, gz; // Integer value from the sensor to be displayed
  float ax_buffer[sample_length];
  float ay_buffer[sample_length];
  float gx_buffer[sample_length];
  float gy_buffer[sample_length];
  float roll_buffer[sample_length];
  float pitch_buffer[sample_length];

  int seq_0 = 0;
  int seq_1 = 0;

  float th_1 = 100;
  float th_2 = 100;

  cmd = 0;
  for (int i = 0;i<sample_length;i++){

      accel.acquire_accel_data_g(accel_data);
      gyro.acquire_gyro_data_dps(gyro_data);

        ax = accel_data[0];
        ay = accel_data[1];
        az = accel_data[2];

        gx = gyro_data[0];
        gy = gyro_data[1];
        gz = gyro_data[2];

        roll_acc = (atan2(-ay,-az)*180.0)/M_PI;
        pitch_acc = (atan2(-ax,sqrt(ay*ay + az*az))*180.0)/M_PI;


        roll = gx*dt+roll;
        pitch = gy*dt+pitch;

        pitch = alpha*pitch+(1-alpha)*pitch_acc;
        roll = alpha*roll+(1-alpha)*roll_acc;

        acc_g_x = -cos(roll/180*M_PI)*sin(pitch/180*M_PI);
        acc_g_y = -sin(roll/180*M_PI)*cos(pitch/180*M_PI);



        ax_modify = ax - acc_g_x;
        if(abs(ax_modify) <= 0.03){
              ax_modify = 0;
              }
        ay_modify = ay - acc_g_y;
        if(abs(ay_modify) <= 0.03){
            ay_modify = 0;
            }

        ax_buffer[i] = ax_modify;
        ay_buffer[i] = ay_modify;
        roll_buffer[i] = roll;
        pitch_buffer[i] = pitch;
        gx_buffer[i] = gx;
        gy_buffer[i] = gy;
        Thread::wait(2);

    }
    /*for (int i = 0;i<sample_length;i++){
    float data_pak[6] = {gx_buffer[i],gy_buffer[i],roll_buffer[i],pitch_buffer[i],ax_buffer[i],ay_buffer[i]};
        if (my_dot(data_pak,coffe_0) > -bias_0) {
            cmd = 1;
            break;
        }else if (my_dot(data_pak,coffe_1) > -bias_1) {
            cmd = 2;
            break;
        }else if (my_dot(data_pak,coffe_2) > -bias_2) {
            cmd = 3;
            break;
        }else if (my_dot(data_pak,coffe_3) > -bias_3) {
            cmd = 4;
            break;
        }
    }*/


    for(int i=0;i<sample_length;i++){
       if(seq_0 == 0){
           if(gx_buffer[i] > 100){
               seq_0 = 1;
            } else if(gx_buffer[i] < -100){
                seq_0 = 2;
            }
        }else {
            if(gx_buffer[i] < -100){
                seq_1 = 1;
                if(seq_0 == seq_1){
                    break;
                }
            }else if(gx_buffer[i] > 100){
                seq_1 = 2;
                if(seq_0 == seq_1){
                    break;
                }
            }
        }
    }
    cmd = 0;
    if(seq_0 == seq_1){
        cmd = seq_0;
    }
    float roll_avg = 0;
    float pitch_avg = 0;
    for (int i=0;i<sample_length;i++){
         roll_avg = roll_avg + roll_buffer[i];
         pitch_avg = pitch_avg + pitch_buffer[i];
    }
    roll_avg = roll_avg/sample_length;
    pitch_avg = pitch_avg/sample_length;


    move_x = 0;
    move_y = 0;

    if ( roll_avg > 10) {
        move_x = roll_avg - 10;
    }
    else if (roll_avg < -10) {
        move_x = roll_avg + 10;
    }
    if (pitch_avg > 10) {
        move_y = pitch_avg - 10;
    }
    else if (pitch_avg < -10) {
        move_y = pitch_avg + 10;
    }

//move_x = seq_0;
//move_y = seq_1;


}
/////////////////////////////////////////////////////

/***********************End of Call Back Functions*****************************/

/********************************Main******************************************/

int main()
{
    /* Register callbacks to application functions */
    kw40z_device.attach_buttonLeft(&ButtonLeft);
    kw40z_device.attach_buttonRight(&ButtonRight);
    kw40z_device.attach_buttonUp(&ButtonUp);
    kw40z_device.attach_passkey(&PassKey);

    /* Turn on the backlight of the OLED Display */
    oled.DimScreenON();

    /* Fills the screen with solid black */
    oled.FillScreen(COLOR_BLACK);

    /* Get OLED Class Default Text Properties */
    oled_text_properties_t textProperties = {0};
    oled.GetTextProperties(&textProperties);

    /* Change font color to Blue */
    textProperties.fontColor   = COLOR_BLUE;
    oled.SetTextProperties(&textProperties);

    /* Display Bluetooth Label at x=17,y=65 */
    strcpy((char *) text,"BLUETOOTH");
    oled.Label((uint8_t *)text,17,65);

    /* Change font color to white */
    textProperties.fontColor   = COLOR_WHITE;
    textProperties.alignParam = OLED_TEXT_ALIGN_CENTER;
    oled.SetTextProperties(&textProperties);

    /* Display Label at x=22,y=80 */
    strcpy((char *) text,"Tap Below");
    oled.Label((uint8_t *)text,22,80);

    uint8_t prevLinkState = 0;
    uint8_t currLinkState = 0;

    // Configure Accelerometer FXOS8700, Magnetometer FXOS8700
    accel.accel_config();
    gyro.gyro_config();

    int cmd = 0;
    float move_x = 0;
    float move_y = 0;

    dataThread.start(dataTask); /*Start transmitting Sensor Tag Data */
 
    while(true) {
       // pc.printf("main while begins %d \r\n", t.read_ms());
      //get_command(cmd,move_x,move_y,20);
      osEvent evt = queue.get();
      //pc.printf("get event status %d \r\n", t.read_ms());
      if (evt.status == osEventMessage) {
        message_t *message = (message_t*) evt.value.p;
        cmd = message->cmd;
        move_x = message->move_x;
        move_y = message->move_y;
        mpool.free(message);


        if (click == true) {
            instruction += 4;
            click = false;
        } else if (cmd == 1) {
            instruction += 6;
        } else if (cmd == 2) {
            instruction += 8;
        }else if (cmd == 3) {
            instruction += 10;
        } else if (cmd == 4) {
            instruction += 12;
        }else if (move_x != 0 || move_y != 0) {
            instruction += 2;
        }
        //pc.printf("main before bluetooth  %d \r\n", t.read_ms());
        blueLed = !kw40z_device.GetAdvertisementMode(); /*Indicate BLE Advertisment Mode*/
        kw40z_device.SendSetApplicationMode(GUI_CURRENT_APP_SENSOR_TAG);

        kw40z_device.SendAccel(instruction,(int16_t)move_x,(int16_t)move_y);
        
        flip = !flip;
        if(flip == true) {
            instruction = 1;
        } else {
            instruction = 0;
        }
        
      }
    Thread::wait(140);
    //pc.printf("main after wait  %d \r\n", t.read_ms());
    }
}


void StartHaptic(void)  {
    hapticTimer.start(50);
    haptic = 1;
}

void StopHaptic(void const *n) {
    haptic = 0;
    hapticTimer.stop();
}

void dataTask(void) {
    t.reset();
    t.start();
    while(true) {
      
      //pc.printf("initilize %d \r\n", t.read_ms());
      int cmd = 0;
      float move_x = 0;
      float move_y = 0;
      int sample_length = 40; 
      
      const float coffe_0[6] = { 3.26058485e-03, -4.89987206e-03, -2.05877017e-02,  3.16671988e-03, 5.79273382e+00, -1.60204263e+00};

      const float coffe_1[6] = { 1.22703505e-03, -2.06662456e-04, -1.68982080e-02,  1.94460189e-02, -4.50079549e+00,  1.09530008e+00};

      const float coffe_2[6] = {-2.84085412e-04, -3.18166798e-03, -7.44055441e-03,  2.84450999e-02, -1.19638051e+00,  5.25903292e+00};

      const float coffe_3[6] = { 7.09153276e-03, -4.21382224e-04, -1.51337127e-02,  5.92974483e-03, 2.22769372e+00, -8.17349130e+00};

      const float bias_0 = -2.069;
      const float bias_1 = -2.005;
      const float bias_2 = -1.976;
      const float bias_3 = -2.617;
      float ax_modify = 0;
      float ay_modify = 0;
      float dt = 0.02;
      float roll = 0;
      float pitch = 0;
      float alpha = 0.6;
      float roll_acc = 0;
      float pitch_acc = 0;
      float acc_g_x = 0; ///acceleration due to gravity
      float acc_g_y = 0;
      float accel_data[3]; // Storage for the data from the sensor
      float ax, ay;//
      float az = 10; // Integer value from the sensor to be displayed
      float gyro_data[3];  // Storage for the data from the sensor
      float gx, gy, gz; // Integer value from the sensor to be displayed
      float ax_buffer[sample_length];
      float ay_buffer[sample_length];
      float gx_buffer[sample_length];
      float gy_buffer[sample_length];
      float roll_buffer[sample_length];
      float pitch_buffer[sample_length];

      int seq_0 = 0;
      int seq_1 = 0;

      float th_1 = 100;
      float th_2 = 100;

     
      //pc.printf("before loop %d \r\n", t.read_ms());
      for (int i = 0;i<sample_length;i++){

          accel.acquire_accel_data_g(accel_data);
          gyro.acquire_gyro_data_dps(gyro_data);

          ax = accel_data[0];
          ay = accel_data[1];
          az = accel_data[2];
          gx = gyro_data[0];
          gy = gyro_data[1];
          gz = gyro_data[2];

          roll_acc = (atan2(-ay,-az)*180.0)/M_PI;
          pitch_acc = (atan2(-ax,sqrt(ay*ay + az*az))*180.0)/M_PI;
          
          roll = gx*dt+roll;
          pitch = gy*dt+pitch;
          pitch = alpha*pitch+(1-alpha)*pitch_acc;
          roll = alpha*roll+(1-alpha)*roll_acc;
          acc_g_x = -cos(roll/180*M_PI)*sin(pitch/180*M_PI);
          acc_g_y = -sin(roll/180*M_PI)*cos(pitch/180*M_PI);

          ax_modify = ax - acc_g_x;
          if(abs(ax_modify) <= 0.03){
                ax_modify = 0;
          }
          ay_modify = ay - acc_g_y;
          if(abs(ay_modify) <= 0.03){
              ay_modify = 0;
          }

          ax_buffer[i] = ax_modify;
          ay_buffer[i] = ay_modify;
          roll_buffer[i] = roll;
          pitch_buffer[i] = pitch;
          gx_buffer[i] = gx;
          gy_buffer[i] = gy;
          Thread::wait(2);
        }
        //pc.printf("after for loop %d \r\n", t.read_ms());
        for(int i=0;i<sample_length;i++){
           if(seq_0 == 0){
               if(gx_buffer[i] > 100){
                   seq_0 = 1;
                } else if(gx_buffer[i] < -100){
                    seq_0 = 2;
                }
           } else {
                if(gx_buffer[i] < -100){
                    seq_1 = 1;
                    if(seq_0 == seq_1){
                        break;
                    }
                }else if(gx_buffer[i] > 100){
                    seq_1 = 2;
                    if(seq_0 == seq_1){
                        break;
                    }
                }
            }
        }
        cmd = 0;
        if(seq_0 == seq_1){
            cmd = seq_0;
        }
        float roll_avg = 0;
        float pitch_avg = 0;
        for (int i=0;i<sample_length;i++){
             roll_avg = roll_avg + roll_buffer[i];
             pitch_avg = pitch_avg + pitch_buffer[i];
        }
        roll_avg = roll_avg/sample_length;
        pitch_avg = pitch_avg/sample_length;


        move_x = 0;
        move_y = 0;

        if ( roll_avg > 10) {
            move_x = roll_avg - 10;
        }
        else if (roll_avg < -10) {
            move_x = roll_avg + 10;
        }
        if (pitch_avg > 10) {
            move_y = pitch_avg - 10;
        }
        else if (pitch_avg < -10) {
            move_y = pitch_avg + 10;
        }
       // pc.printf("before queue %d \r\n", t.read_ms());
        message_t *message = mpool.alloc();
        message->cmd = cmd;
        message->move_x = move_x;
        message->move_y = move_y;
        queue.put(message);
        //pc.printf("after queue %d \r\n", t.read_ms());
        Thread::wait(50);  
       // pc.printf("after wait %d \r\n", t.read_ms());
            
    }
}

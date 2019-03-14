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


const int sample_length = 40;

float ax_buffer[sample_length];
float ay_buffer[sample_length];
float az_buffer[sample_length];
float gx_buffer[sample_length];
float gy_buffer[sample_length];
float roll_buffer[sample_length];
float pitch_buffer[sample_length];

/////classifier parameters.
const float coffe_0[5] = { 0.02846571, -0.00256966, -0.20748284,  0.07996476, -0.06161359};
//{0.02753898, -0.00449275, -0.09590801,  0.1852141,   0.09620012};
const float coffe_1[5] = {-0.02998072,  0.00269631,  0.18283499,  0.00266354,  0.06696428};
//{-0.02837764,  0.00245138,  0.22044746, -0.03473952,  0.05721707};

const float coffe_2[5] = {-0.00093464,  0.02206194, -0.32526807,  0.01250286,  0.31093281};
//{-0.00064494,  0.02229301, -0.09804006,  0.04080106,  0.05151641};

const float coffe_3[5] = { 0.00199224, -0.01681837,  0.02688253, -0.19280518,  0.05341681};
//{ 0.00359614, -0.01951772,  0.08028457, -0.17017418,  0.05402942};
//const float coffe_4[5] = {1.23438e-10, -3.29374637e-10,

const float bias_0 = -4.42310814;
//-3.05663687;
const float bias_1 = -4.56752823;
//-3.09695999;
const float bias_2 = -3.5173094;
//-2.58471739;
const float bias_3 = -3.13541953;
/////////////////////////////////////////////////

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


float my_dot(float a[5], const float b[5]) {
    float result = 0;
    for(int i=0;i<5;i++){
        result = result + a[i]*b[i];
        }
    return result;
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
    int prev_cmd = 0;
    float move_x = 0;
    float move_y = 0;

    dataThread.start(dataTask); /*Start transmitting Sensor Tag Data */

    while(true) {
      osEvent evt = queue.get();
      if (evt.status == osEventMessage) {
        message_t *message = (message_t*) evt.value.p;
        cmd = message->cmd;
        move_x = message->move_x;
        move_y = message->move_y;
        mpool.free(message);
  /////generate instruction from the sensor data
        if (prev_cmd == 0) {
            if (click == true) {
                cmd = 6;
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
        } else {
            instruction += 2;
            cmd = 0;
        }
        prev_cmd = cmd;
        /////////////////////////
        //send data via BLE
        blueLed = !kw40z_device.GetAdvertisementMode(); /*Indicate BLE Advertisment Mode*/
        kw40z_device.SendSetApplicationMode(GUI_CURRENT_APP_SENSOR_TAG);
        kw40z_device.SendAccel((int16_t)instruction,(int16_t)move_x,(int16_t)move_y);
        /////////////////////////////////
        //flip one bit to indicate different package. 
        flip = !flip;
        if(flip == true) {
            instruction = 1;
        } else {
            instruction = 0;
        }
        ///////////////////////////////////////////
      }

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
    while(true) {
//////Variables declearation and init
      int cmd = 0;
      float move_x = 0;
      float move_y = 0;
      float ax_modify = 0;
      float ay_modify = 0;
      float dt = 0.1;
      float roll = 0;
      float pitch = 0;
      float alpha = 0.6;
      float roll_acc = 0;
      float pitch_acc = 0;
      float acc_g_x = 0; ///acceleration due to gravity
      float acc_g_y = 0;
      float accel_data[3]; // Storage for the data from the sensor
      float ax, ay;//
      float az; // Integer value from the sensor to be displayed
      float gyro_data[3];  // Storage for the data from the sensor
      float gx, gy, gz; // Integer value from the sensor to be displayed


      int seq_0 = 0;
      int seq_1 = 0;
      int if_print = 0;

      float th_1 = 100;
      float th_2 = 100;


      int t_0 = t.read_ms();
      //////sample data equal to sample_length and do initial cumputation
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
          ///for debug purpose only
          //pc.printf("%4.2f %4.2f %4.2f %4.2f %4.2f %4.2f %4.2f %4.2f \n" ,gx,gy,gz,1.1,1.1,ax,ay,az);
          ax_buffer[i] = ax;
          ay_buffer[i] = ay;
          az_buffer[i] = az;
          roll_buffer[i] = roll;
          pitch_buffer[i] = pitch;
          gx_buffer[i] = gx;
          gy_buffer[i] = gy;
          //keep the sensors within max sample frequency.
          Thread::wait(2);
        }
        //////////////////////////////////////////////
        int t_1 = t.read_ms();///debug timing.

    seq_0 = 0;
    seq_1 = 0;
    ///more computation for to get the command
    for(int i=0;i<sample_length;i++){
       float data_pak[5] = {gx_buffer[i],gy_buffer[i],ax_buffer[i],ay_buffer[i],az_buffer[i]};
       //float r[4];
       //r[0] = my_dot(data_pak,coffe_0) + bias_0;
       //r[1] = my_dot(data_pak,coffe_1) + bias_1;
       //r[2] = my_dot(data_pak,coffe_2) + bias_2;
       //r[3] = my_dot(data_pak,coffe_3) + bias_3;
       if(abs(ax_buffer[i]) > 1.5 && abs(gx_buffer[i]) < 150 && abs(gy_buffer[i]) < 150){
        //pc.printf("%4.2f %4.2f %4.2f %4.2f %4.2f %4.2f %4.2f %4.2f \n" ,my_dot(data_pak,coffe_0),data_pak[0],data_pak[1],data_pak[2],data_pak[3],data_pak[4],0,0);
          if_print = 1;
           }
       if(seq_0 == 0){
           if(my_dot(data_pak,coffe_0) > -bias_0){
               seq_0 = 1;
            //pc.printf("%4.2f %4.2f %4.2f %4.2f %4.2f %4.2f %4.2f %4.2f \n" ,my_dot(data_pak,coffe_0),data_pak[0],data_pak[1],data_pak[2],data_pak[3],data_pak[4],0,0);
            } else if(my_dot(data_pak,coffe_1) > -bias_1){
                seq_0 = 2;
            } else if (my_dot(data_pak,coffe_2) > -bias_2) {
                seq_0 = 3;
            } else if (my_dot(data_pak,coffe_3) > -bias_3){
                seq_0 = 4;
            }
            //seq_0 = max_indx(r);
        }else {
            if(my_dot(data_pak,coffe_1) > -bias_1){
                seq_1 = 1;
                if(seq_0 == seq_1){
                    break;
                }
            }else if(my_dot(data_pak,coffe_0) > -bias_0){
                seq_1 = 2;
                if(seq_0 == seq_1){
                    break;
                }
            }else if (my_dot(data_pak,coffe_3) > -bias_3){
                seq_1 = 3;
                if(seq_0 == seq_1){
                    break;
                }
            }else if (my_dot(data_pak,coffe_2) > -bias_2){
                seq_1 = 4;
                if(seq_0 == seq_1){
                    break;
                }
            }
        }
    }
/////////////////////////////////////////
        cmd = 0;
        if(seq_0 == seq_1){
            cmd = seq_0;
        }
///handle print exception.
        if(if_print == 1 && cmd == 0){
            cmd = 5;
        }
/////////////////////////////////
        float roll_avg = 0;
        float pitch_avg = 0;
/////get the average value of rotation.
        for (int i=0;i<sample_length;i++){
             roll_avg = roll_avg + roll_buffer[i];
             pitch_avg = pitch_avg + pitch_buffer[i];
        }
        roll_avg = roll_avg/sample_length;
        pitch_avg = pitch_avg/sample_length;


        move_x = 0;
        move_y = 0;
////threshing holding to make it work better
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
//////////////////////////////////
//push data to queue.
        message_t *message = mpool.alloc();
        message->cmd = cmd;
        message->move_x = move_x;
        message->move_y = move_y;
        queue.put(message);
        Thread::wait(200);
/////////////////////////////////
    }
}

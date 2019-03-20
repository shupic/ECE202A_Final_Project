#include "mbed.h"
#include "FXOS8700.h"
#include "Hexi_OLED_SSD1351.h"
#include "images.h"
#include "string.h"
#include "FXAS21002.h"
#include "Hexi_KW40Z.h"

#define M_PI  3.14159265

// Pin connections
KW40Z kw40z_device(PTE24, PTE25);
// RGB LED
DigitalOut led1(LED_GREEN); 
// Serial interface
Serial pc(USBTX, USBRX); 
FXOS8700 accel(PTC11, PTC10);
// Gyroscope
FXAS21002 gyro(PTC11,PTC10); 
v
SSD1351 oled(PTB22,PTB21,PTC13,PTB20,PTE6, PTD15); 

// Variables
Timer t;
//package structure
typedef struct {
  int cmd;
  float move_x;
  float move_y;
  //timing Variables, for debug only.
  int t_0;
  int t_1;
} message_t;

void StartHaptic(void);
void StopHaptic(void const *n);

DigitalOut haptic(PTB9);
RtosTimer hapticTimer(StopHaptic, osTimerOnce);
void dataTask(void);

MemoryPool<message_t, 16> mpool;
Queue<message_t, 16> queue;

Thread dataThread;

//data buffer of sample_length
const int sample_length = 40;
float ax_buffer[sample_length];
float ay_buffer[sample_length];
float az_buffer[sample_length];
float gx_buffer[sample_length];
float gy_buffer[sample_length];
float roll_buffer[sample_length];
float pitch_buffer[sample_length];
/***********************************************************/

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
/***********************************************************/

//call beck function for button
void StartHaptic(void)  {
    hapticTimer.start(50);
    haptic = 1;
}

void StopHaptic(void const *n) {
    haptic = 0;
    hapticTimer.stop();
}
/***********************************************************/
//define a dot product function of two array..
float my_dot(float a[],const float b[]) {
    float result = 0.00;
    for(int i=0;i < sizeof(a);i++){
        result = result + (a[i]*b[i]);
        }
    return result;
    }
/***********************************************************/
//define a function find the index of max elements
//if max element greater than 0..
int max_indx(float a[]){
    int r = 0;
    int max = 0;
    for (int i=0;i<sizeof(a);i++){
        if (a[i] > max) {
            max = a[i];
            r = i;
            }
        }
    return r;
}
/***********************************************************/
bool click = false;

void ButtonUp(void)
{
    StartHaptic();
    //redLed = !redLed;
    click = true;
}


int main(){
  //start timmer for debug purpose 
    t.start();
    t.reset();
/***********************************************************/
    int cmd = 0;
    int prev_cmd = 0;
    float move_x = 0;
    float move_y = 0;
    int t_0;
    int t_1;
    //config sensor and button
    kw40z_device.attach_buttonUp(&ButtonUp);
    accel.accel_config();
    gyro.gyro_config();
/***********************************************************/
    //start the data thread 
    //Start transmitting Sensor Tag Data */
    dataThread.start(dataTask); 

    while(true) {
    int str = t.read_ms();
      //get_command(cmd,move_x,move_y,20);
      osEvent evt = queue.get();
    int bef_if = t.read_ms();
      if (evt.status == osEventMessage) {
        message_t *message = (message_t*) evt.value.p;
        cmd = message->cmd;
        move_x = message->move_x;
        move_y = message->move_y;
        t_0 = message->t_0;
        t_1 = message->t_1;
        mpool.free(message);
        //generating instruction based on sensor data
        int instruction = 0;
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
            }else if (cmd == 5) {
                instruction += 14;
            }
            else if (move_x != 0 || move_y != 0) {
                instruction += 2;
            }
        } else {
            instruction += 2;
            cmd = 0;
        }
        prev_cmd = cmd;
/***********************************************************/
        int bef_print = t.read_ms();
        pc.printf("%4.2f %4.2f %4.2f %4.2f %4.2f %4.2f %4.2f %4.2f \n" ,(float)instruction,(float)bef_print,(float)bef_if,move_x,move_y,(float)t_0,(float)t_1,1.3);
      }
    }
}



void dataTask(void) {
    while(true) {
      //Variables declearation and init
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
      //acceleration due to gravity
      float acc_g_x = 0; 
      float acc_g_y = 0;
      // Storage for the data from the sensor
      float accel_data[3]; 
      // Integer value from the sensor to be displayed
      float ax, ay;
      float az; 
      // Storage for the data from the sensor
      float gyro_data[3];  
      // Integer value from the sensor to be displayed
      float gx, gy, gz; 


      int seq_0 = 0;
      int seq_1 = 0;
      int if_print = 0;

      float th_1 = 100;
      float th_2 = 100;


      int t_0 = t.read_ms();
      //sample data equal to sample_length and do initial cumputation
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
/***********************************************************/
        //debug timing.
        int t_1 = t.read_ms();

    seq_0 = 0;
    seq_1 = 0;
    //more computation for to get the command
    for(int i=0;i<sample_length;i++){
       float data_pak[5] = {gx_buffer[i],gy_buffer[i],ax_buffer[i],ay_buffer[i],az_buffer[i]};
       if(abs(ax_buffer[i]) > 1.3 && abs(gx_buffer[i]) < 150 && abs(gy_buffer[i]) < 150){
          if_print = 1;
           }
       if(seq_0 == 0){
           if(my_dot(data_pak,coffe_0) > -bias_0){
               seq_0 = 1;
            } else if(my_dot(data_pak,coffe_1) > -bias_1){
                seq_0 = 2;
            } else if (my_dot(data_pak,coffe_2) > -bias_2) {
                seq_0 = 3;
            } else if (my_dot(data_pak,coffe_3) > -bias_3){
                seq_0 = 4;
            }
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
/***********************************************************/
        cmd = 0;
        if(seq_0 == seq_1){
            cmd = seq_0;
        }
        //handle print exception. 
        if(if_print == 1 && cmd == 0){
            cmd = 5;
        }
/***********************************************************/
        float roll_avg = 0;
        float pitch_avg = 0;
        //get the average value of rotation.
        for (int i=0;i<sample_length;i++){
             roll_avg = roll_avg + roll_buffer[i];
             pitch_avg = pitch_avg + pitch_buffer[i];
        }
        roll_avg = roll_avg/sample_length;
        pitch_avg = pitch_avg/sample_length;


        move_x = 0;
        move_y = 0;
        //threshing holding to make it work better
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
/***********************************************************/
        //push data to queue.
        message_t *message = mpool.alloc();
        message->cmd = cmd;
        message->move_x = move_x;
        message->move_y = move_y;
        message->t_0 = t_0;
        message->t_1 = t_1;
        queue.put(message);
/***********************************************************/
    }
}

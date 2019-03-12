#include "mbed.h"
#include "FXOS8700.h"
#include "Hexi_OLED_SSD1351.h"
#include "images.h"
#include "string.h"
#include "FXAS21002.h"
#define M_PI  3.14159265
//#include <math.h>

// Pin connections
DigitalOut led1(LED_GREEN); // RGB LED
Serial pc(USBTX, USBRX); // Serial interface
FXOS8700 accel(PTC11, PTC10);
FXAS21002 gyro(PTC11,PTC10); // Gyroscope
SSD1351 oled(PTB22,PTB21,PTC13,PTB20,PTE6, PTD15); // SSD1351 OLED Driver (MOSI,SCLK,POWER,CS,RST,DC)

// Variables
Timer t;

typedef struct {
  int cmd;
  float move_x;
  float move_y;
  int t_0;
  int t_1;
} message_t;


void dataTask(void);

MemoryPool<message_t, 16> mpool;
Queue<message_t, 16> queue;

Thread dataThread;


float my_dot(float a[5],const float b[5]) {
    float result = 0.00;
    for(int i=0;i<5;i++){
        result = result + (a[i]*b[i]);
        }
    return result;
    }



void get_command(int& cmd,float& move_x,float& move_y,int sample_length){

  const float coffe_0[5] = {0.02753898, -0.00449275, -0.09590801,  0.1852141,   0.09620012};
  const float coffe_1[5] = {-0.02837764,  0.00245138,  0.22044746, -0.03473952,  0.05721707};

  const float coffe_2[5] = {-0.00064494,  0.02229301, -0.09804006,  0.04080106,  0.05151641};

  const float coffe_3[5] = { 0.00359614, -0.01951772,  0.08028457, -0.17017418,  0.05402942};

  const float bias_0 = -3.05663687;
  const float bias_1 = -3.09695999;
  const float bias_2 = -2.58471739;
  const float bias_3 = -2.40274942;
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
  float az; // Integer value from the sensor to be displayed
  float gyro_data[3];  // Storage for the data from the sensor
  float gx, gy, gz; // Integer value from the sensor to be displayed
  float ax_buffer[sample_length];
  float ay_buffer[sample_length];
  float az_buffer[sample_length];
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
        
        pc.printf("%4.2f %4.2f %4.2f %4.2f %4.2f %4.2f %4.2f %4.2f \n" ,gx,gy,gz,1.1,1.1,ax,ay,az);
        
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
        az_buffer[i] = az;
        roll_buffer[i] = roll;
        pitch_buffer[i] = pitch;
        gx_buffer[i] = gx;
        gy_buffer[i] = gy;
        Thread::wait(5);

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
       float data_pak[5] = {gx_buffer[i],gy_buffer[i],ax_buffer[i],ay_buffer[i],az_buffer[i]};
       if(seq_0 == 0){
           if(my_dot(data_pak,coffe_0) > -bias_0){
               seq_0 = 1;
            } else if(my_dot(data_pak,coffe_1) > -bias_1){
                seq_0 = 2;
            } else 
            if (my_dot(data_pak,coffe_2) > -bias_2) {
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
    cmd = 0;
    //if(seq_0 == seq_1){
        cmd = gy;
    //}
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


int main(){ 
    t.start();
    t.reset();
    int cmd = 0;
    float move_x = 0;
    float move_y = 0;
    int t_0;
    int t_1;
    
    accel.accel_config();
    gyro.gyro_config();
    
    /*
    while (true) 
    {
      
    get_command(cmd,move_x,move_y,20);
    
          
    //pc.printf("%4.2f %4.2f \n", vx,vy);
    pc.printf("%4.2f %4.2f %4.2f %4.2f %4.2f %4.2f %4.2f %4.2f \n" ,move_x,move_y,(float)cmd,move_x,move_y,1.1,1.1,1.1);
    
      
      Thread::wait(200);
    }*/
    dataThread.start(dataTask); /*Start transmitting Sensor Tag Data */

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

        bool click = false;
        int instruction = 0;
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

        //blueLed = !kw40z_device.GetAdvertisementMode(); /*Indicate BLE Advertisment Mode*/
        //kw40z_device.SendSetApplicationMode(GUI_CURRENT_APP_SENSOR_TAG);

        //kw40z_device.SendAccel(instruction,(int16_t)move_x,(int16_t)move_y);
        int bef_print = t.read_ms();
        pc.printf("%4.2f %4.2f %4.2f %4.2f %4.2f %4.2f %4.2f %4.2f \n" ,(float)cmd,(float)bef_print,(float)bef_if,move_x,move_y,(float)t_0,(float)t_1,1.3);
        //flip = !flip;
        //if(flip == true) {
        //    instruction = 1;
        //} else {
        //    instruction = 0;
        //}

      }
    Thread::wait(1);
    }
}

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


void dataTask(void) {
    while(true) {
      int cmd = 0;
      float move_x = 0;
      float move_y = 0;
      int sample_length = 40;

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
    //-2.40274942;
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
      float ax_buffer[sample_length];
      float ay_buffer[sample_length];
      float az_buffer[sample_length];
      float gx_buffer[sample_length];
      float gy_buffer[sample_length];
      float roll_buffer[sample_length];
      float pitch_buffer[sample_length];

      int seq_0 = 0;
      int seq_1 = 0;

      float th_1 = 100;
      float th_2 = 100;


      int t_0 = t.read_ms();
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
          //pc.printf("%4.2f %4.2f %4.2f %4.2f %4.2f %4.2f %4.2f %4.2f \n" ,gx,gy,gz,1.1,1.1,ax,ay,az);
          ax_buffer[i] = ax_modify;
          ay_buffer[i] = ay_modify;
          az_buffer[i] = az;
          roll_buffer[i] = roll;
          pitch_buffer[i] = pitch;
          gx_buffer[i] = gx;
          gy_buffer[i] = gy;
          Thread::wait(4);
        }
        int t_1 = t.read_ms();
       seq_0 = 0;
       seq_1 = 0; 
    for(int i=0;i<sample_length;i++){
       float data_pak[5] = {gx_buffer[i],gy_buffer[i],ax_buffer[i],ay_buffer[i],az_buffer[i]};
       float r[4];
       r[0] = my_dot(data_pak,coffe_0) + bias_0;
       r[1] = my_dot(data_pak,coffe_1) + bias_1;
       r[2] = my_dot(data_pak,coffe_2) + bias_2;
       r[3] = my_dot(data_pak,coffe_3) + bias_3;
       if(abs(ax_buffer[i]) > 0.7 && abs(gx_buffer[i]) < 150 && abs(gy_buffer[i]) < 150){
        pc.printf("%4.2f %4.2f %4.2f %4.2f %4.2f %4.2f %4.2f %4.2f \n" ,my_dot(data_pak,coffe_0),data_pak[0],data_pak[1],data_pak[2],data_pak[3],data_pak[4],0,0);
           seq_0 = 5;
           seq_1 = 5;
           break;
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
        cmd = 0;
        if(seq_0 == seq_1){
            cmd = seq_0;
        }
        if(cmd == 5){
            Thread::wait(500);
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

        message_t *message = mpool.alloc();
        message->cmd = cmd;
        message->move_x = move_x;
        message->move_y = move_y;
        message->t_0 = t_0;
        message->t_1 = t_1;
        queue.put(message);
        Thread::wait(2);

    }
}

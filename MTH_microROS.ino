#include <micro_ros_arduino.h>
#include <stdio.h>

#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>


#include "BME34M101.h"
#include "BM25S2021-1.h"


#include <std_msgs/msg/Float32.h>
#include <std_msgs/msg/Int32.h>
rcl_publisher_t moisture_publisher temprature_publisher humidity_publisher;
std_msgs__msg__Float32 moisture_msg temprature_msg humidity_msg;
rclc_executor_t executor;
rclc_support_t support;
rcl_allocator_t allocator;
rcl_node_t node;
rcl_timer_t timer;
BM25S2021_1 BMht(&Wire);
BME34M101 mySoilMoistureSensor(5,4);
float temprature, humidity;
uint32_t moisture;
uint8_t moisture_id;
const bool is_farenheit = false;//define to get message in celsius or farenheit
#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){error_loop();}}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){}}


void timer_callback(rcl_timer_t * timer, int64_t last_call_time)
{  
  RCLC_UNUSED(last_call_time);
  if (timer != NULL) {
    humidity = BMht.readTemprature(is_farenheit);
    temprature = BMht.readHumidity();
    moisture = mySoilMoistureSensor.
    humidity_msg.data = humidity;
    temprature_msg.data = temprature;
    moisture_msg.data = moisture;
    RCSOFTCHECK(rcl_publish(&temprature_publisher, &temprature_msg, NULL));
    RCSOFTCHECK(rcl_publish(&humidity_publisher, &humidity_msg, NULL));
    RCSOFTCHECK(rcl_publish(&moisture_publisher, &moisture_msg, NULL));
  }
}

void setup() {
  set_microros_transports();
  
  delay(2000);

  allocator = rcl_get_default_allocator();

  //create init_options
  RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));

  // create node
  RCCHECK(rclc_node_init_default(&node, "farm_omht_sensor", "", &support));

  // create publisher
  RCCHECK(rclc_publisher_init_default(
    &moisture_publisher,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Float32),
    "micro_ros_arduino_node_publisher"));

  RCCHECK(rclc_publisher_init_default(
    &humidity_publisher,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Float32),
    "micro_ros_arduino_node_publisher"));

  RCCHECK(rclc_publisher_init_default(
    &temprature_publisher,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Float32),
    "temprature_sensor"));

  // create timer,
  const unsigned int timer_timeout = 1000;
  RCCHECK(rclc_timer_init_default(
    &timer,
    &support,
    RCL_MS_TO_NS(timer_timeout),
    timer_callback));

  // create executor
  RCCHECK(rclc_executor_init(&executor, &support.context, 1, &allocator));
  RCCHECK(rclc_executor_add_timer(&executor, &timer));

  msg.data = 0;
}

void loop() {
  delay(100);
  RCSOFTCHECK(rclc_executor_spin_some(&executor, RCL_MS_TO_NS(100)));
}


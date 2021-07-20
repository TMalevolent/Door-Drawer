#!/usr/bin/env python
#
# Author: Ryan Roberts
#
# Class for controlling physical Door

import RPi.GPIO as gpio
import sys
import spidev
from time import sleep
from door_data import DataPoint
from rotary_sensor import AS5047P

class Door:
 
    def __init__(self):
        self.angle_sensor = AS5047P(0,0,1000000)
        self.lower_adc = spidev.SpiDev()
        self.lower_adc.open(1, 1) #same bus, synchronous
        self.lower_adc.max_speed_hz = 1000000
        self.upper_adc = spidev.SpiDev()
        self.upper_adc.open(1, 0)
        self.upper_adc.max_speed_hz = 1000000
        #mc pins for reset motor
        self.in3 = 12
        self.in4 = 26
        self.enB = 13
        self.reset_freq = 100
        self.time_unwind = 2 #seconds
        self.reset_dc = 20 #speed of motor
        self.dis_buffer = 1 #buffer value for resetting drawer (in deg)
      
        gpio.setmode(gpio.BCM)
        gpio.setup(self.in3, gpio.OUT)
        gpio.setup(self.in4, gpio.OUT)
        gpio.setup(self.enB, gpio.OUT)
        gpio.output(self.in3, 0)
        gpio.output(self.in4, 0)
        self.reset_pwm = gpio.PWM(self.enB, self.reset_freq)
    
    def __set_friction(self):
        print("set resistance to {}".format(self.__resistance_value))
   
    def __reset_friction(self):
        print("set resistance to 0")
        
    def __read_handle(self):
        data = [-1] * 14
        #lower ADC
        for chan in range(0,8):
            r = self.lower_adc.xfer2([1, 8 + chan << 4, 0])
            data[chan] = int(((r[1] & 3) << 8) + r[2])
        #spidev automatically switches CS signals
        #upper ADC
        for chan in range(0,6):
            r = self.upper_adc.xfer2([1, 8 + chan << 4, 0])
            data[chan + 8] = int(((r[1] & 3) << 8) + r[2])
        return data
  
    def start_new_trial(self, resistance):
        self.__resistance_value = resistance #do calculations to know dc for pwm pin
        self.start_pos = self.angle_sensor.get_angle()
        self.__set_friction()
   
    def collect_data(self):
        data_point = DataPoint(self.start_pos - self.angle_sensor.get_angle(), self.__read_handle())
        return data_point
   
    def reset(self):
        self.__reset_friction()
        did_move = False
        end_pos = self.start_pos - self.dis_buffer
        self.reset_pwm.start(self.reset_dc)
        gpio.output(self.in3, 0)
        gpio.output(self.in4, 1)
        while(True):
            if(self.angle_sensor.get_angle() >= end_pos):
                break
            did_move = True
        if(did_move):
            gpio.output(self.in3, 1)
            gpio.output(self.in4, 0)
            sleep(self.time_unwind)
        gpio.output(self.in3, 0)
        gpio.output(self.in4, 0)
        self.reset_pwm.stop()
     
    def __del__(self):
        gpio.cleanup()
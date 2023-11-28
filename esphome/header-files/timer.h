/****
Copyright (c) 2023 RebbePod

This library is free software; you can redistribute it and/or modify it 
under the terms of the GNU Lesser GeneralPublic License as published by the Free Software Foundation; 
either version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,but WITHOUT ANY WARRANTY; 
without even the impliedwarranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
See the GNU Lesser General Public License for more details. 
You should have received a copy of the GNU Lesser General Public License along with this library; 
if not, write tothe Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA, 
or connect to: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
****/

#pragma once
#include <array>

static template_::TemplateSwitch* * switches[] = {&enabled, &sunday, &monday, &tuesday, &wednesday, &thursday, &friday, &saturday, &repeat, &negative_offset};

static template_::TemplateNumber* * numbers[] = {&time_hour, &time_minute, &output, &action, &offset_hour, &offset_minute, &mode};

// the number should match the amount of relays below
const uint8_t num_of_relays = 2;

// match the output number to the position of the relay here
static gpio::GPIOSwitch* * relays[num_of_relays] = {&relay_0, &relay_1};

/***************************************************
*      ***** Timer Format Cheat Guide *****
* **************************************************
*              ***** Switches *****
*   *** Value set to '1' enabled '0' disabled ***
* 0 = Enabled
* 1-7 = Days (sun-sat)
* 8 = Repeat
* 9 = Negative Offset
*               ***** Numbers *****
* 10 = Time hour
* 11 = Time minute
* 12 = Output  {value '0' for the first position of switch in the 'relays' variable}
* 13 = Action  {'0' turn off, '1' turn on, '2' toggle}
* 14 = Offset hour
* 15 = Offset minute
* 16 = Mode    {'0' use time, '1' use sunrise, '2' use sunset} 
****************************************************/

void doRelayAction(uint8_t i, time_t timestamp, bool set_relays) {
    ESP_LOGD("doRelayAction", "------ doRelayAction ran ------");
    if(set_relays == true){
        if(id(global_timer)[i][13] == 2) {
            ESP_LOGD("doRelayAction", "------ toggle ------");
            // toggle relays [position 12]
            (*relays[id(global_timer)[i][12]])->toggle();
        } else {
            ESP_LOGD("doRelayAction", "------ timer number %i ------", i);
            // check if action needed to be done
            if((*relays[id(global_timer)[i][12]])->state != id(global_timer)[i][13]) {
                ESP_LOGD("doRelayAction", "------ set state %i ------", id(global_timer)[i][13]);
                // set relay state [position 12] from timer action [position 13]
                (*relays[id(global_timer)[i][12]])->publish_state(id(global_timer)[i][13]);
            }
        }
    }
    // set last run time for this timer
    id(global_last_run)[i] = timestamp;
    // if repeat was disabled [position 8] deactivate timer [position 0] after running
    if(id(global_timer)[i][8] == 0) {
        id(global_timer)[i][0] = 0;
    }
}

void setTimestamps(uint8_t set_timer = 100) {
    uint8_t i = 0;
    uint8_t loops = 20;
    if(set_timer != 100) {
        i = set_timer;
        loops = set_timer + 1;
        ESP_LOGD("setTimestamps", "------ updating timer %i ------", set_timer + 1);
    } else {
        ESP_LOGD("setTimestamps", "------ updating all timers ------");
    }
    // set variables
    esphome::ESPTime date = id(sntp_time).now();
    struct tm tm;
    time_t timestamp = 0;
    // loop timers
    for(; i < loops; i++) {
        if(id(global_timer)[i][0] == 1) {
            // either repeat is disabled [position 8] or todays day of week is enabled [position 1-7]
            if(id(global_timer)[i][8] == 0 || id(global_timer)[i][0 + date.day_of_week] == 1) {
                ESP_LOGD("setTimestamps", "------ Active Timer %i is set to run today ------", i + 1);
                ESP_LOGD("setTimestamps", "------ timestamp today %lu ------", date.timestamp);
                // check timer mode [position 16] (time, sunrise etc.)
                if(id(global_timer)[i][16] == 0) {
                    ESP_LOGD("setTimestamps", "------ mode is set to time ------");
                    date.hour = date.minute = date.second = 0;                          // uptime time to 00:00:00
                    date.hour = id(global_timer)[i][10];
                    date.minute = id(global_timer)[i][11];
                    tm = date.to_c_tm();
                    timestamp = mktime(&tm);
                    ESP_LOGD("setTimestamps", "------ timestamp timer (time) %lu ------", timestamp);
                } else if(id(global_timer)[i][16] == 1) {
                    ESP_LOGD("setTimestamps", "------ mode is set to sunrise ------");
                    date.hour = date.minute = date.second = 0;                          // uptime time to 00:00:00
                    date.recalc_timestamp_utc();
                    timestamp = id(mysun).sunrise(date, -0.833)->timestamp;
                    ESP_LOGD("setTimestamps", "------ timestamp timer (sunrise) %lu ------", timestamp);
                } else if(id(global_timer)[i][16] == 2) {
                    ESP_LOGD("setTimestamps", "------ mode is set to sunset ------");
                    date.hour = date.minute = date.second = 0;                          // uptime time to 00:00:00
                    date.recalc_timestamp_utc();
                    timestamp = id(mysun).sunset(date, -0.833)->timestamp;
                    ESP_LOGD("setTimestamps", "------ timestamp timer (sunset)) %lu ------", timestamp);
                }
                // check if negative offset [position 9]
                if(id(global_timer)[i][9] == 1) {
                    // deduct offset hour and minutes [position 14-15] from timestamp
                    timestamp -= id(global_timer)[i][14] * 60 * 60;
                    timestamp -= id(global_timer)[i][15] * 60;
                } else {
                    // add offset hour and minutes [position 14-15] to timestamp
                    timestamp += id(global_timer)[i][14] * 60 * 60;
                    timestamp += id(global_timer)[i][15] * 60;
                }
                // set seconds to 0
                timestamp -= timestamp % 60;
                ESP_LOGD("setTimestamps", "------ timestamp after offset %lu ------", timestamp);
                // set to proper timestamp
                id(global_next_run)[i] = timestamp;
            } else {
                // timer not avalible for today set to 0
                id(global_next_run)[i] = 0;
                // ESP_LOGD("setTimestamps", "------ Active Timer %i is not set to run today ------", i + 1);
            }
        } else {       
            // timer disabled set to 0
            id(global_next_run)[i] = 0;
            // ESP_LOGD("setTimestamps", "------ Inactive Timer %i ------", i + 1);
        }
    }
} // setTimestamps

void onInterval() {
    ESP_LOGD("interval", "------ Ran interval ------");
    if(id(override_timer).state == true) {
        return;
    }
    // set variables
    esphome::ESPTime date = id(sntp_time).now();
    date.timestamp -= date.timestamp % 60;   // set seconds to 0
    bool is_missed_timer = false;
    time_t global_missed_timers[20];
    // loop all 20 timer timestamps
    for (uint8_t i = 0; i < 20; i++) {
        if(id(global_next_run)[i] > 0) {   // check if timer is active
            // ESP_LOGD("interval", "------ Timer %i Active ------", i + 1);
            // ESP_LOGD("interval", "------ Current Time %lu Next Run Time %lu Last Run Time %lu ------", date.timestamp, id(global_next_run)[i], id(global_last_run)[i]);
            if(id(global_next_run)[i] == date.timestamp) { // check if time matche
                ESP_LOGD("interval", "------ Timer %i matches current time ------", i + 1);
                doRelayAction(i, date.timestamp, true);
            }
        } else if(id(global_next_run)[i] > 0 && id(global_next_run)[i] < date.timestamp && id(global_last_run)[i] < (date.timestamp - 86340)) {
            // timer time is before current time and the latest time ran is more the 23h59m ago
            ESP_LOGD("interval", "------ Timer %i not matched - Missed timer ------", i + 1);
            // add timestamp to list of missed timers
            global_missed_timers[i] = id(global_next_run)[i];
            is_missed_timer = true;
        } else {
            // ESP_LOGD("interval", "------ Timer %i not active ------", i + 1);
            // time not active
            global_missed_timers[i] = 0;

        }
    }
    if(is_missed_timer == true) {
        ESP_LOGD("interval", "------ is_missed_timer is true ------");
        // set variables
        bool temp_relays[num_of_relays];
        uint8_t relay_index;
        uint8_t index[20] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19};
        // sort array to earliest to latest got from https://stackoverflow.com/questions/46382252/sort-array-by-first-item-in-subarray-c
        std::sort(index, index + 20, [&](uint8_t n1, uint8_t n2){ return global_missed_timers[n1] < global_missed_timers[n2]; });
        // set the current values of the relays
        for (uint8_t i = 0; i < num_of_relays; i++) {
            temp_relays[i] = (*relays[i])->state;
            // ESP_LOGD("interval", "------ Set default state for relay %i ------", i);
        }
        // set the tem relays with the result of the timer sequence
        for (uint8_t i = 0; i < 20; i++) {
            // ESP_LOGD("interval", "------ loop index %i ------", index[i]);
            // use the sorted index location so we do the actions in the order the timer was set for them
            if(global_missed_timers[index[i]] > 0) {
                // do action without really setting relays
                doRelayAction(index[i], date.timestamp, false);
                ESP_LOGD("interval", "------ index %i is set do action ------", index[i]);
                // set relay index based on output from current timer
                relay_index = id(global_timer)[index[i]][12];
                if(id(global_timer)[index[i]][13] == 2) {
                    // toggle
                    if(temp_relays[relay_index] == 0) {
                        temp_relays[relay_index] = 1;
                    } else {
                        temp_relays[relay_index] = 0;
                    }
                } else {
                    // set state
                    temp_relays[relay_index] = id(global_timer)[index[i]][13];
                }
            }
        }
        // loop relays and set state
        for (uint8_t i = 0; i < num_of_relays; i++) {
            ESP_LOGD("interval", "------ Set relay %i to state %i ------", i, temp_relays[i]);
            if((*relays[i])->state != temp_relays[i]) {
                (*relays[i])->publish_state(temp_relays[i]);
            }
        }
        
    }
} // onInterval

void onSelect(std::string x) {
    ESP_LOGD("onSelect", "------ Ran onSelect ------");
    // ESP_LOGD("onSelect", "%s", x.c_str());
    uint8_t num_timer = 0;
    if(x == "-- Select --") {
        num_timer = 0;
    } else if(x == "Timer 1") {
        num_timer = 1;
    } else if(x == "Timer 2") {
        num_timer = 2;
    } else if(x == "Timer 3") {
        num_timer = 3;
    } else if(x == "Timer 4") {
        num_timer = 4;
    } else if(x == "Timer 5") {
        num_timer = 5;
    } else if(x == "Timer 6") {
        num_timer = 6;
    } else if(x == "Timer 7") {
        num_timer = 7;
    } else if(x == "Timer 8") {
        num_timer = 8;
    } else if(x == "Timer 9") {
        num_timer = 9;
    } else if(x == "Timer 10") {
        num_timer = 10;
    } else if(x == "Timer 11") {
        num_timer = 11;
    } else if(x == "Timer 12") {
        num_timer = 12;
    } else if(x == "Timer 13") {
        num_timer = 13;
    } else if(x == "Timer 14") {
        num_timer = 14;
    } else if(x == "Timer 15") {
        num_timer = 15;
    } else if(x == "Timer 16") {
        num_timer = 16;
    } else if(x == "Timer 17") {
        num_timer = 17;
    } else if(x == "Timer 18") {
        num_timer = 18;
    } else if(x == "Timer 19") {
        num_timer = 19;
    } else if(x == "Timer 20") {
        num_timer = 20;
    }
    ESP_LOGD("onSelect", "------ Timer %i Selected ------", num_timer);
    // ESP_LOGD("onSelect", "------ Timer %i last run at %lu next run at %lu ------", num_timer, id(global_last_run)[num_timer - 1], id(global_next_run)[num_timer - 1]);
    if(num_timer > 0) {
        // set num_timer to be base on position starting from 0
        num_timer -= 1;
        // set all the switches states
        for (uint8_t i = 0; i < 10; i++) {
            (*switches[i])->publish_state(id(global_timer)[num_timer][i]);
            // ESP_LOGD("onSelect", "------ Loop # %i value %i (switch) ------", i, id(global_timer)[num_timer][i]);
        }
        // set all the Number states
        for (uint8_t i = 0; i < 7; i++) {
            (*numbers[i])->publish_state(id(global_timer)[num_timer][10 + i]);
            // ESP_LOGD("onSelect", "------ Loop # %i value %i (number) ------", 10 + i, id(global_timer)[num_timer][10 + i]);
        }
    } else {
        // no timer set all switches off
        for (uint8_t i = 0; i < 10; i++) {
            (*switches[i])->publish_state(false);
        }
        // no timer set all numbers to 0
        for (uint8_t i = 0; i < 7; i++) {
            (*numbers[i])->publish_state(0);
        }
    }
} // onSelect

void onPressSave() {
    ESP_LOGD("onPressSave", "------ Save Button Pressed ------");
    uint8_t num_timer = 0;
    if(id(select_timer).state == "Timer 1") {
        num_timer = 1;
    } else if(id(select_timer).state == "Timer 2") {
        num_timer = 2;
    } else if(id(select_timer).state == "Timer 3") {
        num_timer = 3;
    } else if(id(select_timer).state == "Timer 4") {
        num_timer = 4;
    } else if(id(select_timer).state == "Timer 5") {
        num_timer = 5;
    } else if(id(select_timer).state == "Timer 6") {
        num_timer = 6;
    } else if(id(select_timer).state == "Timer 7") {
        num_timer = 7;
    } else if(id(select_timer).state == "Timer 8") {
        num_timer = 8;
    } else if(id(select_timer).state == "Timer 9") {
        num_timer = 9;
    } else if(id(select_timer).state == "Timer 10") {
        num_timer = 10;
    } else if(id(select_timer).state == "Timer 11") {
        num_timer = 11;
    } else if(id(select_timer).state == "Timer 12") {
        num_timer = 12;
    } else if(id(select_timer).state == "Timer 13") {
        num_timer = 13;
    } else if(id(select_timer).state == "Timer 14") {
        num_timer = 14;
    } else if(id(select_timer).state == "Timer 15") {
        num_timer = 15;
    } else if(id(select_timer).state == "Timer 16") {
        num_timer = 16;
    } else if(id(select_timer).state == "Timer 17") {
        num_timer = 17;
    } else if(id(select_timer).state == "Timer 18") {
        num_timer = 18;
    } else if(id(select_timer).state == "Timer 19") {
        num_timer = 19;
    } else if(id(select_timer).state == "Timer 20") {
        num_timer = 20;
    }
    if(num_timer != 0) {
        // set num_timer to be base on position starting from 0
        num_timer -= 1;
        // set the 10 switches states
        for (uint8_t i = 0; i < 10; i++) {
            id(global_timer)[num_timer][i] = (*switches[i])->state;
            // ESP_LOGD("onPressSave", "------ save Loop Number %i value %i (switch) ------", i, id(global_timer)[num_timer][i]);
        }
        // set the 7 number states (in the global array we start at 10)
        for (uint8_t i = 0; i < 7; i++) {
            id(global_timer)[num_timer][10 + i] = (*numbers[i])->state;
            // ESP_LOGD("onPressSave", "------ save Loop Number %i value %d (number) ------", 10 + i, (*numbers[i])->state);
        }
    }
    // timer settings changed reset timestamps
    setTimestamps(num_timer);
} // onPressSave
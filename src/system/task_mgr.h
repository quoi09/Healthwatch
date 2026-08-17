#ifndef TASK_MGR_H
#define TASK_MGR_H

void system_init();

void task_manager_init();

void sensor_task(void *pv);

void processing_task(void *pv);

void location_task(void *pv);

void alert_task(void *pv);

void display_task(void *pv);

void firebase_task(void *pv);

void power_task(void *pv);

#endif
#ifndef TASK_APP_H_
#define TASK_APP_H_

void init_task(void);
void task_run(void);
void task_emergency_shutdown(void);

void task_page_next(void);
void task_toggle_power_output(void);
void task_adjust_voltage_up(void);
void task_adjust_voltage_down(void);
void task_adjust_current_up(void);
void task_adjust_current_down(void);

#endif /* TASK_APP_H_ */

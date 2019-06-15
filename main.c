#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <linux/can.h>
#include <linux/can/raw.h>

#include "lib.h"


#define NO_OF_MENU_OPTIONS (6+1)
#define CAN_NETWORK_NAME "slcan0" //define it to be a specific name for testing

enum Warning_lights{
	AIRBAGS_WARN = 0x0,
	ENG_CHECK_WARN = 0x1,
	ENG_OIL_WARN = 0x2,
	SEATBELT_WARN = 0x3,
	ABS_WARN = 0x4,
	PARK_BRAKE_WARN = 0x5,
	BATT_WARN = 0x6,
	END_OF_WARN
};

struct _warning_lights_options {
	int option_id;
	char *option_string;
	int can_msg_id;
	int can_msg_len;
	int heartbeat_bytes[4];
	int byte_to_turn_on;
	int byte_to_turn_off;
	int byte_position;
	int send_intervals_ms;
};

typedef int Can_socket;

struct _main_menu_options{
	int option_id;
	void (*function_to_execute)(Can_socket);
	char *menu_string;
};

struct _warning_lights_options warning_lights_options_array[2] = {
	{AIRBAGS_WARN, "[0] Airbag warning light\n", 0x39, 0x2, {0x39, 0x0C, 0x1B, 0x2A}, 0x00, 0x00, 0x00, 20},
	{END_OF_WARN, NULL, 0x0, 0x0, {0x0, 0x0, 0x0, 0x0}, 0x00, 0x00, 0x00, 0}
};

void user_change_warn_lights(Can_socket s){

	int user_on_off_option, user_warn_lights_option;

	struct canfd_frame warning_light_frame; /*TODO: Is there a need to malloc space for this?*/
	int enable_canfd = 1;
	int mtu;
	int required_mtu = CAN_MTU; /**/
	int max_can_byte;

	printf("\n\n\nTODO: Implement how to turn on/off warning lights\n\n\n");

	/*TODO: Ask user to choose what lights to turn on/off*/

	printf( "\nWhich warning light?\n");
	int i = 0;
	do{
		printf("%s", warning_lights_options_array[i].option_string);
		i++;
	} while(warning_lights_options_array[i].option_string != NULL);
	printf("Your choice: ");

	if(scanf("%d", &user_warn_lights_option) <= 0){
		printf("Illegal input.\n");
		return;
	}
	if(user_warn_lights_option < 0 || user_warn_lights_option > 1){
		/*Invalid option*/
		printf("Illegal option.\n");
		return;
	}

	printf("\nTurn on or off?\n"
		"[0] Off\n"
		"[1] On\n"
		"Your choice: ");
	if(scanf("%d", &user_on_off_option) <= 0){
		printf("Illegal input.\n");
		return;
	}
	if(user_on_off_option < 0 || user_on_off_option > 1){
		/*Invalid option*/
		printf("Illegal option.\n");
		return;
	}
	/*TODO: Prepare can frame to be sent*/
	struct _warning_lights_options *pntr_to_selected_warn_light = &(warning_lights_options_array[user_warn_lights_option]);
	warning_light_frame.can_id = pntr_to_selected_warn_light->can_msg_id;
	warning_light_frame.len = pntr_to_selected_warn_light->can_msg_len;
	warning_light_frame.data[pntr_to_selected_warn_light->byte_position] = (user_on_off_option == 1) ? pntr_to_selected_warn_light->byte_to_turn_on : pntr_to_selected_warn_light->byte_to_turn_off;
	
	int heartbeat_index = 0;

	/*TODO: Prepare the heartbeat*/
	for(int i = 0; i < 20000; i++){
		warning_light_frame.data[pntr_to_selected_warn_light->can_msg_len-1] = pntr_to_selected_warn_light->heartbeat_bytes[heartbeat_index];
		if (write(s, &warning_light_frame, required_mtu) != required_mtu) {
			perror("write");
		}
		usleep(pntr_to_selected_warn_light->send_intervals_ms * 1000);
		heartbeat_index++;
		heartbeat_index %= 4;
	}
}

void user_change_rpm(Can_socket s){
	printf("\n\n\nTODO: Implement how to change rpm\n\n\n");
}

void user_change_speed(Can_socket s){

	struct canfd_frame speed_frame; /*TODO: Is there a need to malloc space for this?*/
	int enable_canfd = 1;
	int mtu;
	int required_mtu = CAN_MTU; /**/
	int max_can_byte;

	int user_defined_speed;

	printf("\n\n\nTODO: Implement how to change the speed\n\n\n");	

	printf("Please specify the speed you want to display: ");
	if(scanf("%d", &user_defined_speed) <= 0){
		printf("Illegal input.\n");
		return;
	}	
	if(user_defined_speed < 0 || user_defined_speed > 300){
		/*Invalid option*/
		printf("Invalid speed.\n");
		return;
	}

	max_can_byte = (int) (user_defined_speed * 80000 / 621); /*TODO: Fix the formula*/

	/*set up the can data frame*/
	/*set timer interrupt to send packet every 10ms*/
	/*increament the speed on the can data frame until the specified speed*/
	/* TODO: Make sense of the initialisation from cansend */

	
	memset(&speed_frame, 0, sizeof(speed_frame));
	speed_frame.can_id = 0x158;
	speed_frame.len = CAN_MAX_DLEN;

	int heartbeat_bytes[4];
	int heartbeat_index = 0;

	/*TODO: for loop to increment/decrement the speed*/
	for(int i = 0; i < max_can_byte/*TODO: To replace with whatever bytes correspond to the speed*/; i++){
		if(heartbeat_index==0){
			int first_hb_byte = (rand() % 10) + 0x20;
			int sec_hb_byte = (rand() % 10) + 0x30;
			int third_hb_byte = (rand() % 10) + 0x0;
			int fourth_hb_byte = (rand() % 10) + 0x10;
			heartbeat_bytes[0] = first_hb_byte;
			heartbeat_bytes[1] = sec_hb_byte;
			heartbeat_bytes[2] = third_hb_byte;
			heartbeat_bytes[3] = fourth_hb_byte;
		}
		speed_frame.data[0] = (i & 0xFF00) >> 8;
		speed_frame.data[1] = i & 0xFF;
		speed_frame.data[4] = (i & 0xFF00) >> 8;
		speed_frame.data[5] = i & 0xFF;
		speed_frame.data[7] = heartbeat_bytes[heartbeat_index];
		if (write(s, &speed_frame, required_mtu) != required_mtu) {
			perror("write");
		}
		usleep(10 * 1000);
		heartbeat_index++;
		heartbeat_index %= 4;
	}

	for(int i = 0; i < 500; i++){
		if (write(s, &speed_frame, required_mtu) != required_mtu) {
			perror("write");
		}
		usleep(10 * 1000);
	}

	/*TODO: Find a way to keep it at the specified speed until user quits*/


}

void user_change_gear(Can_socket s){
	printf("\n\n\nTODO: Implement how to change the displayed gear\n\n\n");

	/*TODO: Ask user what gear he wants to spoof.*/

	struct canfd_frame gear_frame; /*TODO: Is there a need to malloc space for this?*/
	int enable_canfd = 1;
	int mtu;
	int required_mtu = CAN_MTU; /**/
	int max_can_byte;

	memset(&gear_frame, 0, sizeof(gear_frame));
	gear_frame.can_id = 0x188;
	gear_frame.len = 6;

	int heartbeat_bytes[4] = {0x34, 0x07, 0x16, 0x22};
	int heartbeat_index = 0;
	gear_frame.data[3] = 0x01; /*TODO: P gear for testing*/
	gear_frame.data[0] = 0x07; /*TODO: D gear for testing*/
	gear_frame.data[2] = 0x80;
	for(int i = 0; i < 20000/*TODO: 20 sec for testing */; i++){
		/*
		if(heartbeat_index==0){
			int first_hb_byte = (rand() % 10) + 0x00;
			int sec_hb_byte = (rand() % 10) + 0x00;
			int third_hb_byte = (rand() % 10) + 0x10;
			int fourth_hb_byte = (rand() % 10) + 0x10;
			int fifth_hb_byte = (rand() % 10) + 0x20;
			int sixth_hb_byte = (rand() % 10) + 0x20;
			int seventh_hb_byte = (rand() % 10) + 0x30;
			heartbeat_bytes[0] = first_hb_byte;
			heartbeat_bytes[1] = sec_hb_byte;
			heartbeat_bytes[2] = third_hb_byte;
			heartbeat_bytes[3] = fourth_hb_byte;
			heartbeat_bytes[4] = fifth_hb_byte;
			heartbeat_bytes[5] = sixth_hb_byte;
			heartbeat_bytes[6] = seventh_hb_byte;
		}
		*/

		gear_frame.data[5] = heartbeat_bytes[heartbeat_index];
		if (write(s, &gear_frame, required_mtu) != required_mtu) {
			perror("write");
		}
		usleep(10 * 1000);
		heartbeat_index++;
		heartbeat_index %= 4;
	}

}

void quit_program(Can_socket s){
	/*TODO: End program*/
	exit(0);
}

void increment_odometer(Can_socket s){
	struct canfd_frame odometer_frame; /*TODO: Is there a need to malloc space for this?*/
	int enable_canfd = 1;
	int mtu;
	int required_mtu = CAN_MTU; /**/
	int max_can_byte;

	printf("\n\n\nTODO: Implement how to increase the odometer\n\n\n");	

	/*set up the can data frame*/
	/*set timer interrupt to send packet every 10ms*/
	/*increament the speed on the can data frame until the specified speed*/
	/* TODO: Make sense of the initialisation from cansend */

	
	memset(&odometer_frame, 0, sizeof(odometer_frame));
	odometer_frame.can_id = 0x158;
	odometer_frame.len = CAN_MAX_DLEN;

	int heartbeat_bytes[4];
	int heartbeat_index = 0;


	for(int repeat = 0; repeat < 10; repeat++){
		for(int i = 0; i <= 0xFF /*TODO: To replace with whatever bytes correspond to the speed*/; i++){
			for(int j = 0; j < 20; j++){
				if(heartbeat_index==0){
					int first_hb_byte = (rand() % 10) + 0x20;
					int sec_hb_byte = (rand() % 10) + 0x30;
					int third_hb_byte = (rand() % 10) + 0x0;
					int fourth_hb_byte = (rand() % 10) + 0x10;
					heartbeat_bytes[0] = first_hb_byte;
					heartbeat_bytes[1] = sec_hb_byte;
					heartbeat_bytes[2] = third_hb_byte;
					heartbeat_bytes[3] = fourth_hb_byte;
				}
				odometer_frame.data[6] = i & 0xFF;
				odometer_frame.data[7] = heartbeat_bytes[heartbeat_index];
				if (write(s, &odometer_frame, required_mtu) != required_mtu) {
					perror("write");
				}
				usleep(10 * 1000);
				heartbeat_index++;
				heartbeat_index %= 4;
			}
		}
	}
}

struct _main_menu_options main_menu_options_array[NO_OF_MENU_OPTIONS] = {
	{0x0, user_change_warn_lights, "[0] Turn on/off warning lights\n"},
	{0x1, user_change_rpm, "[1] Change RPM value\n"},
	{0x2, user_change_speed, "[2] Change the speed\n"},
	{0x3, user_change_gear, "[3] Change the gear\n"},
	{0x4, increment_odometer, "[4] Increment the odometer\n"},
	{0x5, quit_program, "[5] Exit\n"},
	{0x6, NULL, NULL}
};

static int print_main_menu(void){
	int i = 0;

	printf( "\nWhat would you like to do?\n");
	do{
		printf("%s", main_menu_options_array[i].menu_string);
		i++;
	} while(main_menu_options_array[i].menu_string != NULL);
	printf("Your choice: ");
}

int main(int argc, char **argv){
	int user_option = 0;
	int return_value;
	char tmp_chr;
	void *function_to_execute;

	struct sockaddr_can addr;
	struct ifreq ifr;
	
	Can_socket s; /* can raw socket */


	if ((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
		perror("socket");
		fprintf(stderr, "Error with socket.\n");
		exit(1);
	}

	strncpy(ifr.ifr_name, CAN_NETWORK_NAME, IFNAMSIZ - 1);
	ifr.ifr_name[IFNAMSIZ - 1] = '\0';
	ifr.ifr_ifindex = if_nametoindex(ifr.ifr_name);
	if (!ifr.ifr_ifindex) {
		perror("if_nametoindex");
		exit(1);
	}

	memset(&addr, 0, sizeof(addr));
	addr.can_family = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;
	setsockopt(s, SOL_CAN_RAW, CAN_RAW_FILTER, NULL, 0);

	if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("bind");
		exit(1);
	}

	while(1/*program should run forever*/ ){
		print_main_menu();
		if(scanf("%d", &user_option) <= 0){
			printf("Illegal input.\n");
			exit(1);
		}	
		if(user_option < 0 || user_option > (NO_OF_MENU_OPTIONS-1)){
			/*Invalid option*/
			printf("Illegal option.\n");
			continue;
		}
		main_menu_options_array[user_option].function_to_execute(s);
		/*print options for user to choose from*/	
		/*process user options*/
	}
}

static void display_speed(int speed){
	/*TODO: This function is supposed to fork a new thread that will send 158# CAN msg*/

}

#define send_can_msg(s, frame,len) \
	write(s, frame, len)
	/*TODO: This function would be responsible for sending a CAN msg*/


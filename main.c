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

typedef int Can_socket;

struct _main_menu_options{
	int option_id;
	void (*function_to_execute)(Can_socket);
	char *menu_string;
};

void user_change_warn_lights(Can_socket s){
	printf("\n\n\nTODO: Implement how to turn on/off warning lights\n\n\n");
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
		sleep(0.010);
		heartbeat_index++;
		heartbeat_index %= 4;
	}

	for(int i = 0; i < 500; i++){
		if (write(s, &speed_frame, required_mtu) != required_mtu) {
			perror("write");
		}
		sleep(0.010);
	}

	/*TODO: Find a way to keep it at the specified speed until user quits*/


}

void user_change_gear(Can_socket s){
	printf("\n\n\nTODO: Implement how to change the displayed gear\n\n\n");

	struct canfd_frame speed_frame; /*TODO: Is there a need to malloc space for this?*/
	int enable_canfd = 1;
	int mtu;
	int required_mtu = CAN_MTU; /**/
	int max_can_byte;
	int user_defined_speed;

	memset(&speed_frame, 0, sizeof(speed_frame));
	speed_frame.can_id = 0x188;
	speed_frame.len = 6;

	int heartbeat_bytes[4] = {0x33, 0x06, 0x15, 0x24};
	int heartbeat_index = 0;

	/*TODO: for loop to increment/decrement the speed*/
	for(int i = 0; i < 2000/*TODO: 20 sec for testing */; i++){
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
		speed_frame.data[3] = 0x01; /*TODO: P gear for testing*/
		speed_frame.data[5] = heartbeat_bytes[heartbeat_index];
		if (write(s, &speed_frame, required_mtu) != required_mtu) {
			perror("write");
		}
		sleep(0.010);
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
			for(int j = 0; j < 200; j++){
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
				sleep(0.010);
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

	printf( "What would you like to do?\n");
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


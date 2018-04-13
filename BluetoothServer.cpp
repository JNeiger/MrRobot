#include "BluetoothServer.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

BluetoothServer::BluetoothServer() {}
BluetoothServer::~BluetoothServer() {
	close(them);
	close(me);
}
	
void BluetoothServer::scanForDevices() {
	inquiry_info *ii = NULL;
    int max_rsp, num_rsp;
    int dev_id, sock, len, flags;
    int i;
    char addr[19] = { 0 };
    char name[248] = { 0 };

    dev_id = hci_get_route(NULL);
    sock = hci_open_dev( dev_id );
    if (dev_id < 0 || sock < 0) {
        perror("opening socket");
        exit(1);
    }

    len  = 8;
    max_rsp = 255;
    flags = IREQ_CACHE_FLUSH;
    ii = (inquiry_info*)malloc(max_rsp * sizeof(inquiry_info));

    num_rsp = hci_inquiry(dev_id, len, max_rsp, NULL, &ii, flags);
    if( num_rsp < 0 ) perror("hci_inquiry");

    for (i = 0; i < num_rsp; i++) {
        ba2str(&(ii+i)->bdaddr, addr);
        memset(name, 0, sizeof(name));
        if (hci_read_remote_name(sock, &(ii+i)->bdaddr, sizeof(name), 
            name, 0) < 0)
        strcpy(name, "[unknown]");
        printf("%s  %s\n", addr, name);
    }

    free( ii );
    close( sock );
}

void BluetoothServer::listenForClient() {
	uint alen;
	struct sockaddr_rc addr;
	
	if ( (me = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM)) < 0) {
		perror("socket didn't intialize");
		exit(1);
	}
	
	bdaddr_t ADDR_ANY = {0,0,0,0,0,0};
	addr.rc_family = AF_BLUETOOTH;
	addr.rc_bdaddr = ADDR_ANY;
	addr.rc_channel = htobs(4);
	alen = sizeof(addr);
	
	if (bind(me, (struct sockaddr*)&addr, alen) < 0) {
		perror("bind failed");
		exit(1);
	}
	
	listen(me, 10); //10?
	printf("Waiting for connections...\n\r");
	them = accept(me, (struct sockaddr *)&addr, &alen);
	printf("Accepted Client\n\r");
}

int BluetoothServer::readData(char* buffer, int len) {
	int bytes_read = read(them, buffer, len);
	return bytes_read;
}

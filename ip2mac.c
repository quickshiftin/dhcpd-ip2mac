/**
 * @note on compilation
 * gcc -o ip2mac ip2mac.c -ldhcpctl -lomapi -ldst -I.
 */
#include "ip2mac.h"

/**
 * first command line argument is the ip address to get information for
 */
int main (int argc, char **argv)
{
    dhcpctl_data_string ipaddrstring = NULL;
    dhcpctl_data_string value        = NULL;
    dhcpctl_handle connection        = NULL;
    dhcpctl_handle lease             = NULL;
    struct in_addr convaddr;
    isc_result_t status, waitstatus;  /* results of dhcpctl method calls */
    char curMacChar[1];               /* a single character */
    char clientMacAddress[18] = "\0"; /* the clients mac address */
    
    /* make sure we have an ip to check for w/ the dhcp server
     * that means the first command line arg needs to be the ip addr
     */
    if(argc != 2) {
        fprintf(stderr, "Usage: ip2mac <ip-lease to test on dhcp server>");
        exit(0);
    }
    
    dhcpctl_initialize ();                                       /* initialize the dhcpctl library */
    status = dhcpctl_connect(&connection, "127.0.0.1", 7911, 0); /* attempt to connect to the dhcp server on localhost */
    if(status != ISC_R_SUCCESS)                                  /* status on connection */
        exit(0);
    
    dhcpctl_new_object(&lease, connection, "lease"); /* create a new dhcpctl object */
    memset(&ipaddrstring, 0, sizeof(ipaddrstring));  /* allocate memory for an ip address */
    inet_pton(AF_INET, argv[1], &convaddr);          /* convert the address from a string to an AF_INET representation */
    omapi_data_string_new(&ipaddrstring, 4, MDL);    /* format for dhcpctl */
    memcpy(ipaddrstring->value, &convaddr.s_addr, 4); 
    
    status = dhcpctl_set_value (lease, ipaddrstring, "ip-address");
    
    /* status on connection */
    if(status != ISC_R_SUCCESS)
        exit(0);

    status = dhcpctl_open_object(lease, connection, DHCPCTL_EXCL);
    if(status != ISC_R_SUCCESS)
        exit(0);
    
    status = dhcpctl_wait_for_completion(lease, &waitstatus);
    if(status != ISC_R_SUCCESS)
        exit(0);

    /* server not authoritative */
    if(waitstatus != ISC_R_SUCCESS)
        exit(0);

    status = dhcpctl_data_string_dereference(&ipaddrstring, MDL);
    status = dhcpctl_get_value(&value, lease, "hardware-address");

    /* extract the mac address from the result */
    if(status == ISC_R_SUCCESS) {

        /* build a string representation of the mac address */
        int curMacCharIndex;
        for(curMacCharIndex = 0; curMacCharIndex < value->len; curMacCharIndex++) {
            if(curMacCharIndex > 0)
                strcat(clientMacAddress, ":");

            sprintf(curMacChar, "%02x", value->value [curMacCharIndex]);
            strcat(clientMacAddress, curMacChar);
        }        

        /* print the mac address */
        printf(clientMacAddress);
    }
}

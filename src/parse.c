//
// Created by rabiddog on 10/17/25.
//

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#include "common.h"
#include "parse.h"

void list_employees(struct dbheader_t *header, struct employee_t *employees){
	int i = 0;
	for(; i < header->count; i++){
		printf("Employee %d\n", i);
		printf("\tName: %s\n", employees[i].name);
		printf("\tAddress: %s\n", employees[i].address);
		printf("\tHours: %d\n", employees[i].hours);
	}
}

int add_employee(struct dbheader_t *header, struct employee_t **employees, char *addstring){

	if(NULL == header) return STATUS_ERROR;
	if(NULL == employees) return STATUS_ERROR;
	if(NULL == *employees) return STATUS_ERROR;
	if(NULL == addstring) return STATUS_ERROR;

    char *name = strtok(addstring, ",");
	if(NULL == name) return STATUS_ERROR;
	printf("%s\n", name);

    char *address = strtok(NULL, ",");
	if(NULL == address) return STATUS_ERROR;
	printf("%s\n", address);

    char *hours = strtok(NULL, ",");
	if(NULL == hours) return STATUS_ERROR;

    struct employee_t *e = *employees;
    e = realloc(e, sizeof(struct employee_t) * (header->count + 1));

    if(e == NULL){
        printf("Realloc failed to add employee\n");
        return STATUS_ERROR;
    }

	header->count++;

	int currentIndex = header->count -1;
    strncpy(e[currentIndex].name, name, sizeof(e[currentIndex].name) -1);
    strncpy(e[currentIndex].address, address, sizeof(e[currentIndex].address) - 1);
    e[currentIndex].hours = atoi(hours);

	*employees = e;

    return STATUS_SUCCESS;
}

int create_db_header(struct dbheader_t **headerOut){
    struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));

    if(header == NULL){
        printf("Calloc failed to create db header\n");
        perror("calloc");
        return STATUS_ERROR;
    }

    header->version = 1;
    header->count = 0;
    header->magic = HEADER_MAGIC;
    header->filesize = sizeof(struct dbheader_t);

    *headerOut = header;
    return STATUS_SUCCESS;
}

int validate_db_header(int fd, struct dbheader_t **headerOut){
    if(fd < 0){
        printf("Got a bad FD from the user\n");
        return STATUS_ERROR;
    }

    struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
    if(fd == -1){
        printf("Calloc failed to create db header\n");
        return STATUS_ERROR;
    }

    if(read(fd, header, sizeof(struct dbheader_t)) != sizeof(struct dbheader_t)){
        perror("read");
        free(header);
        return STATUS_ERROR;
    }

    header->version = ntohs(header->version);
    header->count = ntohs(header->count);
    header->magic = ntohl(header->magic);
    header->filesize = ntohl(header->filesize);

    if(header->magic != HEADER_MAGIC){
        printf("Invalid database type\n");
        free(header);
        return STATUS_ERROR;
    }

    if(header->version != 1){
        printf("Invalid header version\n");
        free(header);
        return STATUS_ERROR;
    }

    struct stat dbstat = {0};
    fstat(fd, &dbstat);
    if(header->filesize != dbstat.st_size){
        printf("Corrupted database\n");
        free(header);
        return STATUS_ERROR;
    }

    *headerOut = header;
    return STATUS_SUCCESS;
}

void output_file(int fd, struct dbheader_t *dbhdr, struct employee_t *employees){
    if(fd < 0){
        printf("Got a bad FD from the user\n");
        return;
    }

    int header_count = dbhdr->count;

    dbhdr->magic = htonl(dbhdr->magic);
    dbhdr->filesize = htonl(
        sizeof(struct dbheader_t) + header_count * sizeof(struct employee_t)
    );
    dbhdr->count = htons(dbhdr->count);
    dbhdr->version = htons(dbhdr->version);

    lseek(fd, 0, SEEK_SET);
    write(fd, dbhdr, sizeof(struct dbheader_t));

    int i = 0;

    for(; i < header_count; i++){
        employees[i].hours = htonl(employees[i].hours);
        write(fd, &employees[i], sizeof(struct employee_t));
    }

    return;
}

int read_employees(int fd, struct dbheader_t *header, struct employee_t **employeesOut){
    if(fd < 0){
        printf("Got a bad FD from the user\n");
        return STATUS_ERROR;
    }

    int count = header->count;
    struct employee_t *employees = calloc(count, sizeof(struct employee_t));

    if(employees == NULL){
        printf("Calloc failed to create employee array\n");
        return STATUS_ERROR;
    }

    read(fd, employees, count * sizeof(struct employee_t));

    int i = 0;
    for(; i < count; i++){
        employees[i].hours = ntohl(employees[i].hours);
    }

    *employeesOut = employees;
    return STATUS_SUCCESS;
}
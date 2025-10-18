//
// Created by rabiddog on 10/17/25.
//

#ifndef PARSE_H
#define PARSE_H

#define HEADER_MAGIC 0x4c4c4144
#define NAME_LENGTH 256
#define ADDRESS_LENGTH 256

struct dbheader_t {
    unsigned int magic;
    unsigned short version;
    unsigned short count;
    unsigned int filesize;
};

struct employee_t {
    char name[NAME_LENGTH];
    char address[ADDRESS_LENGTH];
    unsigned int hours;
};

int create_db_header(struct dbheader_t **headerOut);
int validate_db_header(int fd, struct dbheader_t **headerOut);
int read_employees(int fd, struct dbheader_t *dbhdr, struct employee_t **employeesOut);
void output_file(int fd, struct dbheader_t *dbhdr, struct employee_t *employees);
int add_employee(struct dbheader_t *dbhdr, struct employee_t **employees, char *addstring);
void list_employees(struct dbheader_t *dbhdr, struct employee_t *employees);

#endif //PARSE_H
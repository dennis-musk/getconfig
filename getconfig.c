#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

/* 
 * The format is :
 * [section1]
 * param1  = value1
 * param2  = value2
 *
 * [section2]
 * param1  = value1
 * param2  = valule2
 *
 */

#define		MAX_SECTON_SIZE		4096
#define		MAX_NAME_SIZE		40
#define		MAX_VALUE_SIZE		40
#define 	MAX_ROW_SIZE		200

static void itoa(int value, char *string, int radix)  
{  
	char tmp[33];  
	char *tp = tmp;  
	int i;  
	unsigned v;  
	int sign;  
	char * sp;  

	if (radix > 36 || radix <= 1) {   
		return ;   
	}   

	sign = (radix == 10 && value < 0);

	if (sign)  
		v = -value;  
	else  
		v = (unsigned)value;

	while (v || tp == tmp) {
		i = v % radix;
		v = v / radix; 
		if (i < 10) 
			*tp++ = i + '0'; 
		else 
			*tp++ = i + 'a' - 10; 
	}   
	sp = string; 
	if (sign) 
		*sp++ = '-'; 
	while (tp > tmp)  
		*sp++ = *--tp;  
	*sp = '\0';  
} 

static void remove_space(char *str)
{
	char *s1 = str;
	int i;

	for(i=0; str[i] != '\0'; ++i) {
		if(!isblank(str[i])) {
			*s1++ = str[i];
		}   
	}   
	*s1 = '\0';
}

/* get_section - get the whole section string from config file
 * @config_file: config file name, include path
 * @section:	 the section what is looking for from the config file
 * @revalue:	 pointer where the section string store if found in config file
 *
 * return: 0 if success, negative if error happend.
 * */
int get_section(const char *config_file, const char *section, char *revalue)
{
	FILE *fd;
	int start;
	char tmp[MAX_NAME_SIZE] = {0};
	char row[MAX_ROW_SIZE];

	fd = fopen(config_file, "r");
	if (fd == NULL) {
		perror("fopen():");
		return -1;
	}

	start = 0;

	snprintf(tmp, strlen(section) + 1, "[%s]", section);

	memset(row, 0, MAX_ROW_SIZE);
	while (fgets(row, MAX_ROW_SIZE, fd)) {
		remove_space(row);

		if (row[0] == '#' || row[0] == '\n') {
			continue;
		}

		if(row[0] == '[' && start) {
			break;
		}

		if (!strncmp(row, tmp, strlen(section))) {
			start = 1;
		}

		if (start) {
			if (strlen(revalue) + strlen(row) >= MAX_SECTON_SIZE) {
				return -ENOMEM;
			}	

			strncpy(revalue + strlen(revalue), row, strlen(row) + 1);
		}

		memset(row, 0, MAX_ROW_SIZE);
	}

	fclose(fd);
	return 0;
}


int get_value(const char *section, const char *node_name, char *revalue)
{
	int i;
	char *begin, *end, *p;
	char name_cmp[MAX_NAME_SIZE] = {0};
	char value_tmp[MAX_VALUE_SIZE] = {0};

	if (section == NULL || node_name == NULL) {
		printf("The section or node name pointer is null\n");
		return -EINVAL;
	}

	snprintf(name_cmp, strlen(node_name) + 1, "%s", node_name);

	p = value_tmp;

	begin = strstr(section, name_cmp);
	if (begin == NULL) {
		return -EFAULT;
	}

	begin += strlen(name_cmp); /* end of node_name */

	end = strchr(begin, '\n'); 

	while (isblank(*begin) || (*begin) == '=') {
		begin++;	
	} 

	strncpy(value_tmp, begin, end - begin);

	/* remove end blank from value_tmp */
	for (i = 0; i < strlen(value_tmp); ++i) {
		if (isblank(p[i]))
			p[i] = '\0';
	}

	memcpy(revalue, value_tmp, strlen(value_tmp));

	return 0;
}


int write_value(const char *config_file, const char *section, const char *node_name, int value)
{
	FILE *fd, *fd_new;
	char name_tmp[MAX_NAME_SIZE] = {0};
	char name_cmp[MAX_NAME_SIZE] = {0};
	char row[MAX_ROW_SIZE];
	char value_str[33];

	if (section == NULL || node_name == NULL) {
		printf("The section or node name pointer is null\n");
		return -EINVAL;
	}

	/* format the section name as [XXXX] */
	snprintf(name_cmp, strlen(section) + 1, "[%s]", section);

	itoa(value, value_str, 10);

	fd = fopen(config_file, "r+");
	if (fd == NULL) {
		perror("fopen():");
		return -1;
	}

	fd_new = fopen(".tmpfile", "w");
	if (fd == NULL) {
		perror("fopen():");
		return -1;
	}

	memset(row, 0, MAX_ROW_SIZE);

	while (fgets(row, MAX_ROW_SIZE, fd)) {
		remove_space(row);
		if (row[0] == '[') {
			/* save the current section name */
			strncpy(name_tmp, row, strlen(row) + 1);
		}
		if (!strncmp(node_name, row, strlen(node_name))) {
			/* Check the node whethor belong to the section which we
			 * want, as the diffirent section can have
			 * same name nodes number.
			 * */
			if (!strncmp(name_tmp, name_cmp, strlen(name_cmp))) {
				/* set the target value as new value */
				snprintf(row, strlen(node_name) + strlen(value_str) + 3, 
						"%s=%s\n", node_name, value_str);
			}
		}
		
		/* write configure information into file */
		fwrite(row, sizeof(char), strlen(row), fd_new);	
	}

	fclose(fd_new);
	fclose(fd);

	/* remove old configure file */
	remove(config_file);
	/* rename new configure file to original */
	rename(".tmpfile", config_file);

	return 0;
}

int main(void)
{
	int ret;
	char section[MAX_SECTON_SIZE] = {0};
	char value[MAX_VALUE_SIZE] = {0};

	ret = get_section("./commModule.conf", "RcdCfg", section);
	if (ret < 0) {
		perror("get_section()");
		exit(1);
	}

	printf("The section is:\n%s", section);
	ret = get_value(section, "BoardNum", value);
	if (ret < 0) {
		perror("get_value()");
		exit(1);
	}
	printf("The BoardNum is : %s\n", value);
#if 0
	memset(value, 0, MAX_VALUE_SIZE);

	ret = get_value(section, "MaxVoltage", value);
	if (ret < 0) {
		perror("get_value()");
		exit(1);
	}

	printf("The MaxVoltage is : %s\n", value);

	memset(value, 0, MAX_VALUE_SIZE);
	ret = get_value(section, "TimeLength", value);
	if (ret < 0) {
		perror("get_value()");
		exit(1);
	}

	printf("The TimeLength is : %s\n", value);
#endif
	write_value("./commModule.conf", "RcdCfg", "BoardNum", 2);
	write_value("./commModule.conf", "RcdCfg", "TimeLength", 258);
	write_value("./commModule.conf", "RcdCfg", "MaxVoltage", 9999);

	return 0;
}

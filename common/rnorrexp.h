#ifndef RNORREXP_H
#define RNORREXP_H
 
#define RNOR 
#define REXP 

float nfix(void);
float efix(void);

/* The ziggurat method for RNOR and REXP */
void zigset(unsigned long jsrseed);

float rnor();
float rexp();

#endif

// SPDX-License-Identifier: GPL-2.0
/*
 * Nuvoton NPCM7xx Binary Image Generator:   Bingo
 *
 * This tool is a general purpose header builder
 * It is used to create a header descibed in an external
 * xml file. 
 * To add changes to the header: update the external xml only.
 * Bingo can also be used to build an binary image from multiple sources
 * of data: binary files, arrays and const data.
 * 
 * Copyright (C) 2018 Nuvoton Technologies, All Rights Reserved
 */
/*    bingo_types.h 
 *            This file contains NTIL generic types, including compiler and core dependent
 */

#ifndef __BINGO_TYPES_H__
#define __BINGO_TYPES_H__

/*---------------------------------------------------------------------------------------------------------*/
/*                                        GENERIC TYPES DEFINITIONS                                        */
/*---------------------------------------------------------------------------------------------------------*/
typedef unsigned char  UINT8;                       /* Unsigned  8 bit quantity                            */
typedef signed   char  INT8;                        /* Signed    8 bit quantity                            */
typedef unsigned short UINT16;                      /* Unsigned 16 bit quantity                            */
typedef short          INT16;                       /* Signed   16 bit quantity                            */
typedef float          FP32;                        /* Single precision floating point                     */
typedef double         FP64;                        /* Double precision floating point                     */



#if ((~0ULL>>1) == 0x7FFFFFFFLL)
/*-----------------------------------------------------------------------------------------------------*/
/* unsigned long is 32bit (32bit Linux, 32bit Windows and 64bit Windows                                */
/*-----------------------------------------------------------------------------------------------------*/
typedef unsigned long   UINT32;                 /* Unsigned 32 bit quantity                            */
typedef signed   long   INT32;                  /* Signed   32 bit quantity                            */

#else
/*-----------------------------------------------------------------------------------------------------*/
/* unsigned long is 64bit (mostly on 64bit Linux systems)                                              */
/*-----------------------------------------------------------------------------------------------------*/
typedef unsigned int    UINT32;                 /* Unsigned 32 bit quantity                            */
typedef signed   int    INT32;                  /* Signed   32 bit quantity                            */

#endif


/*---------------------------------------------------------------------------------------------------------*/
/* Auxiliary macros (not part of the interface)                                                            */
/*---------------------------------------------------------------------------------------------------------*/
#define _REG_READ_AUX_(addr,type,size)              _REG_ACCESS_FORM1(type,R,size,addr)
#define _REG_WRITE_AUX_(val,addr,type,size)         _REG_ACCESS_FORM2(type,W,size,addr,val)

#define _REG_ACCESS_FORM1(type,rw,size,arg)         type##rw##size((arg))
#define _REG_ACCESS_FORM2(type,rw,size,arg1,arg2)   type##rw##size((arg1), (arg2))
#define _CASTING_FROM(size)                         _CASTING_FROM2(UINT,size)
#define _CASTING_FROM2(type,size)                   type##size

#define _GET_REG(addr,accesstype,size)              addr,accesstype,size
#define _GET_REG_ADDR(addr,accesstype,size)         addr
#define _GET_REG_ACCESSTYPE(addr,accesstype,size)   accesstype
#define _GET_REG_SIZE(addr,accesstype,size)         size

#define _GET_FIELD(pos,size)                        pos,size
#define _GET_FIELD_POS(pos,size)                    pos
#define _GET_FIELD_SIZE(pos,size)                   size


#define _READ_FIELD(reg, reg_size, size, position) \
	(((reg) >> (position)) & ((((_CASTING_FROM(reg_size))1) << (size)) - 1))

#define _SET_REG_FIELD(reg, size, position, value)    \
	REG_WRITE(_GET_REG(reg), (((REG_READ(_GET_REG(reg))) &(~((((_CASTING_FROM(_GET_REG_SIZE(reg)))1 << (size)) - 1) << (position))))\
	| (((((_CASTING_FROM(_GET_REG_SIZE(reg)))1 << (size)) - 1) & (value)) << (position))))

#define  _SET_REG_FIELD_SIZE(reg,size,position,value)                                                                   \
if (size == 1)                                                                                                      \
{                                                                                                                   \
if (value == 1)                                                                                                 \
{                                                                                                               \
	_SET_REG_BIT_FIELD(_GET_REG(reg), position);                                                                \
}                                                                                                               \
	else                                                                                                            \
{                                                                                                               \
	_CLEAR_REG_BIT_FIELD(_GET_REG(reg), position);                                                              \
}                                                                                                               \
}                                                                                                                    \
   else                                                                                                                 \
{                                                                                                                    \
	_SET_REG_FIELD(_GET_REG(reg), size, position, value);                                                                \
}


#define _SET_VAR_FIELD(var,size,position,value)                                                                                                 \
	((sizeof(var) == 4) ? ((var) = ((var)& (~((((UINT32)1 << (size)) - 1) << (position)))) | (((((UINT32)1 << (size)) - 1) & (value)) << (position))) : \
	((sizeof(var) == 2) ? ((var) = ((var)& (~((((UINT16)1 << (size)) - 1) << (position)))) | (((((UINT16)1 << (size)) - 1) & (value)) << (position))) : \
	((var) = ((var)& (~(((1 << (size)) - 1) << (position)))) | ((((1 << (size)) - 1) & (value)) << (position)))))

#define _SET_REG_BIT_FIELD(reg, position)    \
	REG_WRITE(_GET_REG(reg), ((REG_READ(_GET_REG(reg))) | ((_CASTING_FROM(_GET_REG_SIZE(reg)))1 << (position))))

#define _READ_REG_BIT_FIELD(reg, position)    \
	(((reg) >> (position)) & (_CASTING_FROM(_GET_REG_SIZE(reg))1)

#define _CLEAR_REG_BIT_FIELD(reg, position)    \
	REG_WRITE(_GET_REG(reg), ((REG_READ(_GET_REG(reg))) & (~((_CASTING_FROM(_GET_REG_SIZE(reg)))1 << (position)))))

#define _MASK_FIELD(size, position)   \
	(((1UL << (size)) - 1) << (position))

#define _BUILD_FIELD_VAL(size, position, value)   \
	((((1UL << (size)) - 1) & (value)) << (position))



/*---------------------------------------------------------------------------------------------------------*/
/*                      REGISTERS AND VARIABLES ACCESS, BIT AND FIELD OPERATION MACROS                     */
/*---------------------------------------------------------------------------------------------------------*/
/* Generic registers access macros assuming registers database defined as follows: (see Coding Standards   */
/* document Chapter 12 for detailed description)                                                           */
/*                                                                                                         */
/* Module defintition:                                                                                     */
/* ------------------                                                                                      */
/*                                                                                                         */
/* #define <MODULE>_BASE_ADDR          (<module_base_addr> )             these definition should be kept   */
/*                                                                       in  a <chip>.h file, since it is  */
/*                                                                       chip specific                     */
/*                                                                                                         */
/* #define <MODULE>_ACCESS             (<module_access_type> )           same name as access macros        */
/*                                                                       name,i.e MEM, IO, INDEXDATA, etc. */
/*                                                                       The rest of the definitions       */
/*                                                                       should be kept in a <hw_module>.h */
/*                                                                       file since they are module        */
/*                                                                       specific                          */
/*                                                                                                         */
/* Regsiter defintition:                                                                                   */
/* --------------------                                                                                    */
/*                                                                                                         */
/* #define <MY_REG_1>              (<MODULE>_BASE_ADDR + <offset>), <ACCESS TYPE>, <8/16/32>               */
/*                                                                                                         */
/* Fields defintition:                                                                                     */
/* -------------------                                                                                     */
/*                                                                                                         */
/* #define <MY_REG_1_FIELD_1>   <field 1 position>, <field 1 size>                                         */
/* #define <MY_REG_1_FIELD_2>   <field 2 position>, <field 2 size>                                         */
/* ...                                                                                                     */
/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* When there is a definition of registers as described above, manipulations on registers and fields can   */
/* be done in the following ways:                                                                          */
/*                                                                                                         */
/* 1. The registers can be accessed directly using the following macros:                                   */
/*                                                                                                         */
/*          REG_READ(reg)                                                                                  */
/*          REG_WRITE(reg,val)                                                                             */
/*                                                                                                         */
/* 2. Registers field can be accessed using the following macros, whose internal implementation access the */
/*    register using REG_READ and REG_WRITE macors (implicit read-modify-write)                            */
/*                                                                                                         */
/*                                                                                                         */
/*          READ_REG_FIELD(reg, field)                                                                     */
/*          SET_REG_FIELD(reg, field, value)                                                               */
/*                                                                                                         */
/*                                                                                                         */
/* 3. For registers field manipulation via explicit read-modify-write procedure on a variable, use the     */
/*    following macros:                                                                                    */
/*                                                                                                         */
/*          READ_VAR_FIELD(var, field)                                                                     */
/*          SET_VAR_FIELD(var, field, value)                                                               */
/*                                                                                                         */
/*                                                                                                         */
/* 4. For the cases when the fields are single bits and are stored in a variable rather than in a constant,*/
/*    the following sets of macros can be used:                                                            */
/*                                                                                                         */
/*          SET_REG_BIT(reg, bit_no)                                                                       */
/*          READ_REG_BIT(reg, bit_no)                                                                      */
/*          CLEAR_REG_BIT(reg, bit_no)                                                                     */
/*                                                                                                         */
/*          SET_VAR_BIT(var, bit_no)                                                                       */
/*          READ_VAR_BIT(var, bit_no)                                                                      */
/*          CLEAR_VAR_BIT(var, bit_no                                                                      */
/*                                                                                                         */
/* 5. For the cases where masks that should be written to the registers are variables, one may use the     */
/*    the following macros. These macros should be used rarely, only in the cases where the usual method   */
/*    of fields defitinions can not be used:                                                               */
/*                                                                                                         */
/*          SET_VAR_MASK(var, bit_mask)                                                                    */
/*          READ_VAR_MASK(var, bit_mask)                                                                   */
/*          CLEAR_VAR_MASK(var, bit_mask)                                                                  */
/*                                                                                                         */
/*          SET_REG_MASK(reg, bit_mask)                                                                    */
/*          READ_REG_MASK(reg, bit_mask)                                                                   */
/*          CLEAR_REG_MASK(reg, bit_mask)                                                                  */
/*                                                                                                         */
/*                                                                                                         */
/* 6. Sometimes it is required to just get a mask (filled with 1b) that fits a certain field without       */
/*    writing a value to it. For this purpose use the macro                                                */
/*                                                                                                         */
/*          MASK_FIELD(field)                                                                              */
/*                                                                                                         */
/* 7. For building a value of a mask without writing to actual register or value, use:                     */
/*                                                                                                         */
/*          BUILD_FIELD_VAL(field, value)                                                                  */
/*                                                                                                         */
/*    This macro is useful when several fields of register/variable are required to be set at a time.      */
/*    In this case, an ORed chain of BUILD_FIELD_VAL invocations will be written to the                    */
/*    register/variable                                                                                    */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------------------------------*/
/* Get field of a register / variable                                                                      */
/*---------------------------------------------------------------------------------------------------------*/
#define READ_VAR_FIELD(var, field)                                                                         \
	((sizeof(var) == 4) ? (_READ_FIELD(var, 32, _GET_FIELD_SIZE(field), _GET_FIELD_POS(field))) : \
	((sizeof(var) == 2) ? (_READ_FIELD(var, 16, _GET_FIELD_SIZE(field), _GET_FIELD_POS(field))) : \
	(_READ_FIELD(var, 8, _GET_FIELD_SIZE(field), _GET_FIELD_POS(field)))))

/*---------------------------------------------------------------------------------------------------------*/
/* Set field of a register / variable                                                                      */
/*---------------------------------------------------------------------------------------------------------*/
#define SET_VAR_FIELD(var, field, value)                                                                   \
	_SET_VAR_FIELD(var, _GET_FIELD_SIZE(field), _GET_FIELD_POS(field), (value))


/*---------------------------------------------------------------------------------------------------------*/
/* Variable bit operation macros                                                                           */
/*---------------------------------------------------------------------------------------------------------*/
#define SET_VAR_BIT(var, nb)                                                                               \
	((sizeof(var) == 4) ? ((var) |= ((UINT32)0x1 << (nb))) : \
	((sizeof(var) == 2) ? ((var) |= ((UINT16)0x1 << (nb))) : \
	((var) |= (0x1 << (nb)))))

#define READ_VAR_BIT(var, nb)      (((var) >> (nb)) & 0x1)

#define CLEAR_VAR_BIT(var, nb)                                                                             \
	((sizeof(var) == 4) ? ((var) &= (~((UINT32)0x1 << (nb)))) : \
	((sizeof(var) == 2) ? ((var) &= (~((UINT16)0x1 << (nb)))) : \
	((var) &= (~(0x1 << (nb))))))

#define MASK_BIT(nb)               (1UL<<(nb))

/*---------------------------------------------------------------------------------------------------------*/
/* Variable mask operation macros                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
#define SET_VAR_MASK(var, bit_mask)       ((var) |= (bit_mask))

#define READ_VAR_MASK(var, bit_mask)      ((var) &  (bit_mask))

#define CLEAR_VAR_MASK(var, bit_mask)     ((var) &= (~(bit_mask)))

/*---------------------------------------------------------------------------------------------------------*/
/* Build a mask of a register / variable field                                                             */
/*---------------------------------------------------------------------------------------------------------*/
#define MASK_FIELD(field)                                                                                  \
	_MASK_FIELD(_GET_FIELD_SIZE(field), _GET_FIELD_POS(field))

/*---------------------------------------------------------------------------------------------------------*/
/* Expand the value of the given field into its correct position                                           */
/*---------------------------------------------------------------------------------------------------------*/
#define BUILD_FIELD_VAL(field, value)                                                                      \
	_BUILD_FIELD_VAL(_GET_FIELD_SIZE(field), _GET_FIELD_POS(field), (value))


/*---------------------------------------------------------------------------------------------------------*/
/* Getting info about registers and fields                                                                 */
/*---------------------------------------------------------------------------------------------------------*/
#define  REG_ADDR(reg)                _GET_REG_ADDR(reg)
#define  GET_FIELD(pos,size)           pos,size




#endif /* __BINGO_TYPES_H__ */

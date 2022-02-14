# Binary Generator (Bingo)
Generate a Binary from XML with ECC support
Latest version 0.0.5 supports Arbel.

# bingo
bingo - Binary Image Generator

## Requirements (Windows):
1) Microsoft .NET framework 4

## Requirements (Linux):
1) g++ 4.6.3 and up

## Windows version:
  A compiled executable is provided under ./windows
  
## How to Build (Linux):
  make (located at ./linux_build)


## INTRODUCTION

The Bingo is a tool that parse XML file in order to create a binary file from different kind of fields and/or binary files.
It also has the ability to perform Nibble Parity or Majority Rule ECC on selected fields.
##	Purpose
Many times there is a need to generate binary files from several binaries, add few fields to the binaries (headers for example) and calculate ECC on some of the fields.
Example of needs:
•	Binary image for fuse (OTP) array that contain several fields that some of them are binaries themselves, 
and some of the fields needs to be ECC encoded
•	Binary images for flash that add header descriptor for the code
##	Product Definition
The Bingo tool is a command line tool which is written in C++ so it is easy to port it between different OS.
It uses an XML file which describes the output file, and optionally several binary files pointed by the XML file.
##	Relevant Concepts
This section describes basic approaches and new concepts to the Bingo tool.
###	Global Concepts
•	**Nibble Parity ECC**- The figure below illustrates the encoding scheme for nibble parity ECC. 
This encoding scheme allows correction of a single bit error per nibble (4 bits). 
Every nibble of the original data is translated to a byte by adding four ECC bits.
 
The following shows the decoding algorithm for the nibble parity ECC.
Ei designates bit I of the encoded byte. Di designates bit i of the decoded original nibble:
```
• D0 = {E0 & (E1 ^ E4)} | {E0 & (E2 ^ E6)} | {(E1 ^ E4) & (E2 ^ E6)}
• D1 = {E1 & (E0 ^ E4)} | {E1 & (E3 ^ E7)} | {(E0 ^ E4) & (E3 ^ E7)}
• D2 = {E2 & (E0 ^ E6)} | {E2 & (E3 ^ E5)} | {(E0 ^ E6) & (E3 ^ E5)}
• D3 = {E3 & (E2 ^ E5)} | {E3 & (E1 ^ E7)} | {(E2 ^ E5) & (E1 ^ E7)}
```
•	**Majority rule ECC** - the entire field value will be duplicated 3 times one after another (so actual size will be x3 times bigger). In order to decode the data, the user should perform majority rule per bit.
For example, if the original data is 32 bits long, the bits will be decoded as following: 
out[0] = MajRule(in[0], in[0+32], in[0+64]).

#	OVERVIEW OF REQUIREMENTS
This section describes the requirements from the Bingo tool.

##	Features and functions
The Bingo Tool supports the following:

-	Parse specific XML files that contains binary output description.
-	Ability to set the output binary size.
-	Ability to pad the file with a specific byte value.
-	Add content of data into a specific offset and specific size.
-	Content of data can be taken from a text inside the XML.
-	Content of data can be taken from other binary file and with a specific offset inside the file.
-	The offset or size can be set by calculated a size of a file.
-	The offset or size can be aligned up to a specific alignment.

#	INTERFACE
This section describes the interfaces between the product and the entire environment in which it operates.

##	XML interface
The Bingo Tool reads and parses an XML formatted specification file.

##	Example in XML Format

The Bingo Tool reads and parses the following XML format:

```
<!-- Bin_Ecc_Map field is mandatory -->
<Bin_Ecc_Map>    
    <!-- ImageProperties is optional, if omitted the values of its children will be their default -->
    <ImageProperties>
        <!-- BinSize is the size of the output binary,
if BinSize = 0 or it is omitted, the binary size will be calculated oat runtime according to BinField -->
        <BinSize>1024</BinSize>
        <!-- PadValue is the value padded between foelds in the binary or to its end, 
             if PadValue is omitted its default value is 0 -->
        <PadValue>0xFF</PadValue>
    </ImageProperties>
        
    <!-- BinField must appear at list once inside Bin_Ecc_Map -->
    <BinField>
        <!-- name is an optional field that gives a meaning to this BinField -->
        <name>MyFirstField</name>
        <!-- config describes the BinField configuration -->
        <config>
            <!-- ecc describes the ECC method: 
                   none (or if this field is omitted): BinField.content will be added as is,
                     actual size will be config.size x1
                   majority: the BinField.content will be duplicated 3 times one after another, 
                     actual size will be config.size x3
                   nibble: the BinField.content will be calculated nibble by nibble with its nibble parity calculation, 
                     actual size will be config.size x2
                   10_bits_majority: config.size must be 2, but only the first 10 bits are relevant for calculation, 
                     those 10 bits will be duplicated 3 times one after another, actual size will be 4 bytes -->
            <ecc>majority</ecc>
            <!-- offset defines the offset inside the binary file that the BinField.content will be put (in value format type) -->
            <offset>0</offset>
            <!-- size defines the size of BinField.content inside the binary file before it ECC calculation (in value format type)
                 For each ecc calculation the actual size will be: 
                   none:     size x1
                   majority: size x3
                   nibble:   size x2
                   10_bits_majority: 4 bytes -->
            <size>8</size>
        </config>
        <!-- content the user should fill into the binary file (in value format type) -->
        <content format='bytes'>0x01 0x23 0x45 0x67 0x89 0xAB 0xCD 0xEF </content>
    </BinField>
</Bin_Ecc_Map>

<!-- value format types
       The types that can get some binary values are called here value format types
       Those types can have several attributes:
         format='32bit' (default): the text value is considered 32 bit value, 
           if preceded with 0x it is considered hexadecimal value, otherwise a decimal value
           example: <content format='32bit'>0x67452301</content>
         format='bytes' : the text value is considered 8 bit value, 
           if preceded with 0x it is considered hexadecimal value, otherwise a decimal value
           if there are several values one after another with space between the second one will be 1 byte after the first one and so forth
           example: <content format='bytes'>0x50 0x07 0x55 0xAA 0x54 0x4F 0x4F 0x42</content>
         format='FileContent': the text value is considered a path to a file that its content is taken into the field 
           example: <content format='FileContent'>./BootBlock.bin</content>
         format='FileSize': the text value is considered a path to a file that its length is calculated and taken into the field
           example: <size format='FileSize'>./BootBlock.bin</size>
         align='value': if format='FileSize' attribute is used the value of the field will be aligned up to the attribute value
           if preceded with 0x it is considered hexadecimal value, otherwise a decimal value
           example: <size format='FileSize' align='0x1000'>./BootBlock.bin </size>
         file_start_offset='value': if format='FileContent' is used, this attribute determine from which offset to take the content of the file
           without this field the offset is 0 (the size taken is from size field)
           if preceded with 0x it is considered hexadecimal value, otherwise a decimal value
           example: <content format='FileContent' file_start_offset='128'> ./RSA_Public_Key_2.bin</content>
       Tag fields that can take the value format types:
         BinField.config.offset
         BinField.config.size
         BinField.content
reverse='condition': this attribute indicates bingo if the binary field should be reversed when it is implemented into output binary. Valid values are 'true' and 'false'.
           example: <content format='FileContent' reverse ='true'> ./signature.bin</content>
-->
```



##	XML Fields and specifications
Tree Hierarchy
Bingo interpretable XML should have the following hierarchy:
 
## Image Properties Element
The <ImageProperties> XML element is a field that describes global properties of the desired output binary file. 
This element has two children, as described below:
 
**<BinSize>** is the size of the output binary, if BinSize = 0 or it is omitted, the binary size will be calculated at runtime according to the inputs binary fields.

**<PadValue>** is the value padded between the binary components (binary fields) in the output binary image. It is also the value padded from the last binary field to the end of the binary image, if applicable. If PadValue is omitted its default value is 0. This value is limited to 8bits (0x00 – 0xFF).

Binary Field Element
The **<BinField>** element described a single binary component that should be added to the output file.  This element has few children, as described below:
 
**<name>** is an optional field that gives a meaning to this BinField, it is not required, but may help for understanding the tree.

**<config>** describes the BinField configuration as following:
  
-**<ecc>**  is an optional field that described the ECC scheme that shall be used. May be one of the following values: none, majority, nibble, 10_bits_majority.
-	**<offset>** defines the offset inside the binary file that the BinField content will be put. This field type is “value type” and it may include value type attributes (see Value Fields Attribute).
-	**<size>** defines the size of BinField content inside the output binary image. Note: the size reflects the size of the content before applying ECC. This field type is “value type” and it may include value type attributes (see Value Fields Attribute).
-	**<content>** defines the actual data that will be appended to the file. This field type is “value type” and it may include value type attributes (see Value Fields Attribute).


Value Fields Attributes
The BinField element includes some “value type” children nodes. Value type means a numeric value which can be taken from different sources: actual text in the XML node, file content, or a file size. 
The selection between the different kinds of input values is done according to the node attributes described below:

-	**Format** – selects the format in which Bingo should interpret the input. May be one of the following: '32bit', 'bytes', 'FileContent', 'FileSize' (default – ‘32bit’). See detailed explanation about each attribute in section ‎3.1.2.
-	**Alignment** – alignment (in bytes, default = 0) that Bingo should perform on the input value.
-	File Start **Offset** – when the value format is selected to be FileContent, this attribute contains the offset inside that file from which Bingo would start take data from.
-	**Reverse** – A Boolean value (may be ‘true’ or ‘false’, default is ‘false’),  which tells bingo weather to reverse the input data (after ECC encoding).


##	XML Fields and specifications

###	Command line interface
```
bingo.exe <xml_file> [-o <generated_bin_file>]
```

**<xml_file>**	- The XML file that bingo should parse

### Flags:
*-o <generated_bin_file>*	- Generated bin file name (default: bin_image.bin)


### Examples:
```
	bingo.exe poleg_fuse_map.xml
```
This invocation will:
-	Parse polg_fuse_map.xml file fields
-	Generate bingo_output.bin file

```
	bingo.exe poleg_fuse_map.xml –o poleg_fuse.bin
```
	
This invocation will:
-	Parse polg_fuse_map.xml file fields
-	Generate polg_fus.bin file


###	Practical Example
The following example describes the XML and the command line, in which a user would like to make a flash image, which contains a SW header and two SW images at different offsets.

For this example, we will create an XML file: headers_and_code.xml as following:
```
<?xml version="1.0" encoding="UTF-8"?>
<!--- example - an image with header and two possible code images -->
<Bin_Ecc_Map>
    <ImageProperties>
        <BinSize>0</BinSize>         <!—let it be calculated by the tool -->
        <PadValue>0xFF</PadValue>   <!-- Flash image so pad with 0xFF -->
    </ImageProperties>
        
    <BinField>
        <name>Header</name>         <!-- name of field -->
        <config>
            <offset>0</offset>            <!-- offset in the image -->
            <size format='FileSize'>.\header_image.bin</size>              
        </config>
        <content format='FileContent'>.\header_image.bin</content>  
    </BinField>
    
    <BinField>
        <!-- The code binary file -->
        <name>Primary Code</name>             <!-- name of field -->
        <config>
            <offset>0x200</offset>        <!-- offset in the image -->
            <size format='FileSize'>.\code_image1.bin</size
        </config>
        <content format='FileContent'>.\code_image1.bin</content>  
    </BinField> 
    
    <BinField>
        <!-- The second code binary file -->
        <name>Secondary Code</name>             <!-- name of field -->
        <config>
            <offset>0x5200</offset>        <!-- offset in the image -->
            <size format='FileSize'>.\code_image2.bin</size
        </config>
        <content format='FileContent'>.\code_image2.bin</content>  
    </BinField> 
<Bin_Ecc_Map>
```

The command line for this example will be:
```
bingo.exe  headers_and_code.xml  -o flash_image.bin
```
After running this, the user will have a binary file which includes the header image at offset 0, first code image at offset 0x200, and second code image at offset 0x5200. Note: The Tool will assert an error if the size of  Primary Code will overlap with Secondary Code.



### Licence
This application is using pugiXML from https://pugixml.org/ which is provided with the MIT license.

The Bingo tool is provided under GPLv2 License.


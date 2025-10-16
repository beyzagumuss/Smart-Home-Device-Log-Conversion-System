#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlwriter.h>
#include <libxml/xmlschemastypes.h>
#include <json-c/json.h>

void csvToBinary(char fileName[], char outputName[], int seperator, int opsys);
void binaryToXML(char outputName[]);
void validation(char fileName[],char outputName[]);
//create LogEntry
typedef struct
{
    char device_id[10];
    char timestamp[30];
    float temperature;
    int humidity;
    char status[8];
    char location[31];
    char alert_level[10];
    int battery;
    char firmware_ver[15];
    int event_code;
} LogEntry;

//help message
void printHelp()
{
    printf("Usage: ./program inputFile outputFile conversionType -separator <1|2|3> -opsys <1|2|3>\n");
    printf("  -separator: 1 = comma (,), 2 = tab (\\t), 3 = semicolon (;)\n");
    printf("  -opsys:     1 = Windows (\\r\\n), 2 = Linux (\\n), 3 = MacOS (\\r)\n");
    printf("  -h:         Show this help screen\n");
}

int main(int argc, char *argv[])
{
    // if only ""./program -h" is entered
    if (argc == 2 && strcmp(argv[1], "-h") == 0)
    {
        printHelp();
        return 0;
    } 

    if (strcmp(argv[2], "2") && argc < 8)
    {
        printf("Error: Missing arguments.\n\n");
        printHelp();
        return 1;
    } //the input must be "inputFileName outputFileName conversionType -separator number -opsys number", at least 8 arguments.

    int separator = 0;
    int opsys = 0;

    

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-separator") == 0 && i + 1 < argc)
        {
            i++; // check value after -separator text
            if (strcmp(argv[i], "1") == 0)
                separator = 1; // 1 for ","
            else if (strcmp(argv[i], "2") == 0)
                separator = 2; //2 for "\t"
            else if (strcmp(argv[i], "3") == 0)
                separator = 3; //3 for ";"
            else
                separator = -1; //false entry
        }
        else if (strcmp(argv[i], "-opsys") == 0 && i + 1 < argc)
        {
            i++;
            if (strcmp(argv[i], "1") == 0)
                opsys = 1; //1 for windows -> "\r\n"
            else if (strcmp(argv[i], "2") == 0)
                opsys = 2; //2 for linux -> "\n"
            else if (strcmp(argv[i], "3") == 0)
                opsys = 3; //3 for mac -> "\r"
            else
                opsys = -1; //false entry
        }
        else if (strcmp(argv[i], "-h") == 0)
        {
            printHelp(); //print help if -h is written in any position
            return 0;
        }
    }

    char fileName[255];
    strcpy(fileName, argv[1]);  //name of input file (1st name)
    int conversionType;
    char outputName[255];
    strcpy(outputName, argv[2]); //name of output file (2nd name)

    if(strcmp(argv[2], "2") == 0)
    {
        conversionType = 2;
    }
    else
    {
        conversionType = atoi(argv[3]);
    }

    // Check if separator and opsys values entered correctly
    if (separator <= 0 || opsys <= 0)
    {
        printf("Error: Missing or invalid separator or opsys arguments.\n\n");
        printHelp();
        return 1;
    }
    
    switch(conversionType) //call function for the conversion type
    {
        case 1:
            csvToBinary(fileName, outputName, separator, opsys);
            break;
        case 2:
            binaryToXML(fileName);
            break;
        case 3:
            validation(fileName, outputName);
            break;
        default:
            printf("Error: Invalid conversion type.\n");
            printf("1-csv to Binary\n2-Binary to XML\n3-XML Validation\n");
            return 1;
    }

    return 0;
}

void csvToBinary(char fileName[], char outputName[], int separator, int opsys)
{

    FILE *csv_file = fopen(fileName, "r"); //open input csv file
    if (csv_file == NULL)
    {
        printf("Error opening csv.\n");
        return;
    }

    FILE *dat_file = fopen(outputName, "wb");
    if (dat_file == NULL)
    {
        printf("Error opening output binary file.\n");
        fclose(csv_file); //close csv file if cannot open/create dat file.
        return;
    }

    char delimiter_char;

    switch (separator) //assign delimiter_char
    {
    case 1:
        delimiter_char = ',';
        break;
    case 2:
        delimiter_char = '\t';
        break;
    case 3:
        delimiter_char = ';';
        break;
    default:
        printf("Internal Error: Unknown separator type.\n");
        fclose(csv_file); //close files if error occurs
        fclose(dat_file);
        return;
    }
    char lineEnding;
    switch (opsys)
    {
    case 1:
        lineEnding = '\r\n';
        break;
    case 2:
        lineEnding = '\n';
        break;
    case 3:
        lineEnding = '\r';
        break;
    default:
        printf("Internal Error: Unknown opys type.\n");
        fclose(csv_file); //close files if error occurs
        fclose(dat_file);
        return;
    }

    bool isHeader = true; 
    char line[1024];
    char buffer[512];
    int bufferIndex = 0;
    LogEntry entry;
    int fieldIndex = 0;
    while (fgets(line, sizeof(line), csv_file) != NULL) //read csv file line by line
    {
        if (isHeader)
        {
            isHeader = false; //skip header line without reading
            continue;
        }
        fieldIndex = 0;
        for (size_t i = 0; i < strlen(line); i++)
        {
            if (delimiter_char == line[i] || lineEnding == line[i]) //when a delimiter or end of line character is reached
            {
                buffer[bufferIndex]='\0'; //stop adding characters to the buffer
                switch (fieldIndex) //which column of this line we are reading
                {
                case 0: // device_id
                    strncpy(entry.device_id, buffer, sizeof(entry.device_id) - 1); //do not copy more than 9 characters
                    entry.device_id[sizeof(entry.device_id) - 1] = '\0'; //make last byte end character to show string ends
                    break;
                case 1: // timestamp
                    strncpy(entry.timestamp, buffer, sizeof(entry.timestamp) - 1);
                    entry.timestamp[sizeof(entry.timestamp) - 1] = '\0'; 
                    break;
                case 2: // temperature
                    entry.temperature = (strlen(buffer) > 0) ? atof(buffer) : 0.0; //if buffer is empty assign 0.0 to temperature
                    break;
                case 3: // humidity
                    entry.humidity = (strlen(buffer) > 0) ? atoi(buffer) : 0;
                    break;
                case 4: // status
                    strncpy(entry.status, buffer, sizeof(entry.status) - 1);
                    entry.status[sizeof(entry.status) - 1] = '\0';
                    break;
                case 5: // location
                    strncpy(entry.location, buffer, sizeof(entry.location) - 1);
                    entry.location[sizeof(entry.location) - 1] = '\0'; 
                    break;
                case 6: // alert_level
                    strncpy(entry.alert_level, buffer, sizeof(entry.alert_level) - 1);
                    entry.alert_level[sizeof(entry.alert_level) - 1] = '\0'; 
                    break;
                case 7: // battery
                    entry.battery = (strlen(buffer) > 0) ? atoi(buffer) : 0;
                    break;
                case 8: // firmware_ver
                    strncpy(entry.firmware_ver, buffer, sizeof(entry.firmware_ver) - 1);
                    entry.firmware_ver[sizeof(entry.firmware_ver) - 1] = '\0';
                    break;
                case 9: 
                    entry.event_code = (strlen(buffer) > 0) ? atoi(buffer) : 0;
                    break;
                default:
                    printf("Error: Unknown field in csv file.\n");
                    break;
                }
                fieldIndex++; //read next column
                bufferIndex = 0;
                buffer[0] = '\0'; //buffer is empty string now
                if (lineEnding == line[i])
                {
                    break; //go to next line
                }
            }
            else //if delimiter or end of line is not reached yet
            {
                buffer[bufferIndex] = line[i]; //add character to buffer
                bufferIndex++;
            }
        }
        fwrite(&entry, sizeof(LogEntry), 1, dat_file); //write entry to binary file
    }
    printf("CSV to Binary conversion is completed successfully.\n");
}

void binaryToXML(char outputName[])
{
    struct json_object *parsed_json;
    struct json_object *dataFileName, *keyStart, *keyEnd, *order;

    FILE *fp_json = fopen("setupParams.json", "r"); //open json file
    if (fp_json == NULL)
    {
        printf("Error: 'setupParams.json' file cannot be opened.\n");
        return;
    }

    fseek(fp_json, 0, SEEK_END); //go to end of file
    long json_size = ftell(fp_json); //find size of file
    rewind(fp_json); //go back to start of file
    char *json_buffer = malloc(json_size + 1); //use json_size to allocate memory
    fread(json_buffer, 1, json_size, fp_json); //read all data in file and store it in buffer
    json_buffer[json_size] = '\0'; //end the buffer with end character
    fclose(fp_json); //close file

    parsed_json = json_tokener_parse(json_buffer); //convert data to json objects by parsing
    free(json_buffer); //buffer is not needed anymore

    json_object_object_get_ex(parsed_json, "dataFileName", &dataFileName);
    json_object_object_get_ex(parsed_json, "keyStart", &keyStart);
    json_object_object_get_ex(parsed_json, "keyEnd", &keyEnd);
    json_object_object_get_ex(parsed_json, "order", &order);

    //assign json objects to variables
    const char *filename = json_object_get_string(dataFileName); 
    int kStart = json_object_get_int(keyStart);
    int kEnd = json_object_get_int(keyEnd);
    const char *sortOrder = json_object_get_string(order);

    //read binary file logdata.dat (given as input argument)
    FILE *fp_bin;
    fp_bin = fopen(filename, "rb"); //open in read binary mode
    if (fp_bin == NULL)
    {
        printf("Error: %s file cannot be opened.\n", filename);
        return;
    }

    fseek(fp_bin, 0, SEEK_END); //go to end of file
    long bin_size = ftell(fp_bin); //find size of file
    rewind(fp_bin); //go back to start of file

    int entry_count = bin_size / sizeof(LogEntry); //calculate number of entries (divide size of file by size of a single entry)

    LogEntry *entries = malloc(bin_size); //allocate file sized memory 

    size_t entries_read = fread(entries, sizeof(LogEntry), entry_count, fp_bin); //load entries from binary file to allocated memory

    fclose(fp_bin); //close binary file

    //apply bubble sort algorithm for ASC and DESC options
    for (int i = 0; i < entry_count - 1; i++)
    {
        for (int j = 0; j < entry_count - 1 - i; j++)
        {
            // compare elements based on key field
            size_t key_length = kEnd - kStart + 1;

            //compare a field to the field next to it in memory using memcmp function
            int comparison_result = memcmp(
                (const char *)&entries[j] + kStart,
                (const char *)&entries[j + 1] + kStart,
                key_length);

            bool swap = false;
            if (strcmp(sortOrder, "DESC"))
            {
                // for DESC option swap if current is less than next
                if (comparison_result < 0)
                {
                    swap = true;
                }
            }
            else
            {
                // for ASC option swap if current is greater than next
                if (comparison_result > 0)
                {
                    swap = true;
                }
            }

            // swap elements if true
            if (swap)
            {
                LogEntry temp = entries[j];
                entries[j] = entries[j + 1];
                entries[j + 1] = temp;
            }
        }
    }

    xmlDocPtr doc = NULL;        
    xmlNodePtr root_node = NULL; 

    //get the root element name from the output argument 
    char root_element_name[256]; //buffer for name
    strncpy(root_element_name, outputName, sizeof(root_element_name) - 1);
    root_element_name[sizeof(root_element_name) - 1] = '\0'; //null termination

    //remove .xml from the name
    char *dot = strrchr(root_element_name, '.');
    if (dot && strcmp(dot, ".xml") == 0)
    {
        *dot = '\0'; //Truncate the string at the dot
    }

    //create new xml document
    doc = xmlNewDoc(BAD_CAST "1.0");

    //create root node with output name
    root_node = xmlNewNode(NULL, BAD_CAST root_element_name);

    xmlDocSetRootElement(doc, root_node); // set root element

    //create nodes by looping through each element read from binary file
    for (int i = 0; i < entry_count; i++)
    {
        // create an <entry> node for each "LogEntry" record
        xmlNodePtr entry_node = xmlNewChild(root_node, NULL, BAD_CAST "entry", NULL);

        // add id attribute to the node (starting from i+1 = 1)
        char id_str[20]; // buffer for id attribute value
        sprintf(id_str, "%d", i + 1);
        xmlNewProp(entry_node, BAD_CAST "id", BAD_CAST id_str);

        // add child nodes for each <entry> (LogEntry)

        // <device> 
        xmlNodePtr device_node = xmlNewChild(entry_node, NULL, BAD_CAST "device", NULL);
        if (device_node)
        { 
            xmlNewChild(device_node, NULL, BAD_CAST "device_id", BAD_CAST entries[i].device_id);
            xmlNewChild(device_node, NULL, BAD_CAST "location", BAD_CAST entries[i].location);
            xmlNewChild(device_node, NULL, BAD_CAST "firmware_ver", BAD_CAST entries[i].firmware_ver);
        }

        // <metrics> 
        xmlNodePtr metrics_node = xmlNewChild(entry_node, NULL, BAD_CAST "metrics", NULL);
        if (metrics_node)
        { 
            xmlNewProp(metrics_node, BAD_CAST "status", BAD_CAST entries[i].status);
            xmlNewProp(metrics_node, BAD_CAST "alert_level", BAD_CAST entries[i].alert_level);

            char temp_str[20];  //buffer for temperature
            sprintf(temp_str, "%.2f", entries[i].temperature); //format "0.00"
            xmlNewChild(metrics_node, NULL, BAD_CAST "temperature", BAD_CAST temp_str);

            char humidity_str[10]; // buffer for humidity 
            sprintf(humidity_str, "%d", entries[i].humidity);
            xmlNewChild(metrics_node, NULL, BAD_CAST "humidity", BAD_CAST humidity_str);

            char battery_str[10]; // buffer for battery 
            sprintf(battery_str, "%d", entries[i].battery);
            xmlNewChild(metrics_node, NULL, BAD_CAST "battery", BAD_CAST battery_str);
        }

        // <timestamp>
        //timestamp will be assigned a dummy value if it is empty
        const char* timestamp_value = (entries[i].timestamp && strlen(entries[i].timestamp) > 0) ? entries[i].timestamp : "2000-01-01T00:00:00";

        xmlNewChild(entry_node, NULL, BAD_CAST "timestamp", BAD_CAST timestamp_value);

        //<event_code>
        char event_code_value_str[20];
        sprintf(event_code_value_str, "%d", entries[i].event_code); //decimal value as data

        xmlNodePtr event_code_node = xmlNewChild(entry_node, NULL, BAD_CAST "event_code", BAD_CAST event_code_value_str); 
        if (event_code_node)
        { 
            // Calculate and add attributes: hexBig, hexLittle, decimalFromHexLittle

            int event_val = entries[i].event_code; // The decimal value of event_code

            // buffers for hexadecimal strings
            char hex_big_endian[9] = ""; 
            char hex_little_endian[9] = "";

            unsigned char *byte_ptr = (unsigned char *)&event_val; 

            //little endian: LSB to MSB
            sprintf(hex_little_endian, "%02X%02X%02X%02X",
                    byte_ptr[0], byte_ptr[1], byte_ptr[2], byte_ptr[3]); 

            
            //big endian: MSB to LSB (reverse read)
            sprintf(hex_big_endian, "%02X%02X%02X%02X",
                    byte_ptr[3], byte_ptr[2], byte_ptr[1], byte_ptr[0]); 

            //decimal
            char decimal_str[20];
            sprintf(decimal_str, "%d", entries[i].event_code);
            
            // added as attributes to <event_code>
            xmlNewProp(event_code_node, BAD_CAST "hexBig", BAD_CAST hex_big_endian);
            xmlNewProp(event_code_node, BAD_CAST "hexLittle", BAD_CAST hex_little_endian);
            xmlNewProp(event_code_node, BAD_CAST "decimal", BAD_CAST decimal_str);
        }
    }

    // save XML document to output file (given as argument)
    int xml_write_status = xmlSaveFormatFileEnc(outputName, doc, "UTF-8", 1); 

    if (xml_write_status == -1)
    {
        fprintf(stderr, "Error writing XML to file %s\n", outputName);
    }
    else
    {
        printf("Binary to XML conversion is completed succesfully.\n");
    }

    // free the XML document and nodes
    xmlFreeDoc(doc);

    // free the allocated memory for entries
    free(entries);
}

void validation(char fileName[],char outputName[])
{

    xmlDocPtr doc;
    xmlSchemaPtr schema = NULL;
    xmlSchemaParserCtxtPtr ctxt;

    char *XMLFileName = fileName; 
    char *XSDFileName = outputName; 

    xmlLineNumbersDefault(1);                   // set line numbers, 0> no substitution, 1>substitution
    ctxt = xmlSchemaNewParserCtxt(XSDFileName); // create an xml schemas parse context
    schema = xmlSchemaParse(ctxt);              // parse a schema definition resource and build an internal XML schema
    xmlSchemaFreeParserCtxt(ctxt);              // free the resources associated to the schema parser context

    doc = xmlReadFile(XMLFileName, NULL, 0); // parse an XML file
    if (doc == NULL)
    {
        fprintf(stderr, "Could not parse %s\n", XMLFileName);
    }
    else
    {
        xmlSchemaValidCtxtPtr ctxt; // structure xmlSchemaValidCtxt, not public by API
        int ret;

        ctxt = xmlSchemaNewValidCtxt(schema);  // create an xml schemas validation context
        ret = xmlSchemaValidateDoc(ctxt, doc); // validate a document tree in memory
        if (ret == 0)                          // validated
        {
            printf("%s validates\n", XMLFileName);
        }
        else if (ret > 0) // positive error code number
        {
            printf("%s fails to validate\n", XMLFileName);
        }
        else // internal or API error
        {
            printf("%s validation generated an internal error\n", XMLFileName);
        }
        xmlSchemaFreeValidCtxt(ctxt); // free the resources associated to the schema validation context
        xmlFreeDoc(doc);
    }
    // free the resource
    if (schema != NULL)
        xmlSchemaFree(schema); // deallocate a schema structure

    xmlSchemaCleanupTypes(); // cleanup the default xml schemas types library
    xmlCleanupParser();      // cleans memory allocated by the library itself
    xmlMemoryDump();         // memory dump
}
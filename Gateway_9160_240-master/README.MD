 #### Bluetooth:AWS IOT Core Integration with Gateway Firmware(Asset Tracker)
   ___


In this project, **Asset tracker V2** used as the base project with some more features added

__1. Features :__

  * LTE/NB-IoT
  * AWS IoT Configured
  * MQTT Protocol between AWS IoT Core and Gateway via LTE
  * Central Add its own parameters in Received packet from Peripheral and finally forward to Gateway(nrf9160) via BLE 5.0
  * Gateway on Receiving Packet from Central forwards it to AWS IoT Core via LTE
  * AWS Cloud, first read it to AWS IoT Core,then stores it to AWS Dynamodb through Lambda     Function

__2. Docklight Code Startup Data (UART0)__

  [Code Startup Data](cloud_connectivity.png)

__3. Docklight Data from Central (UART1)__

[Data from Central](central_packet.png)

 __4. Startup requirement:__
 
  * NCS SDK version 2.4.0 
  * VS code Studio
  * Docklight
  * NCS extension
  * Github
  * nrf9160DK/Custom Board
  * AWS Cloud Account 

__5. Files included in this project:__

  * uart_1_event.c
  * uart_1_event.h
  * uart_1_module.c
  
#### 6. Certificate 
>Update the following way:\
>AT+CFUN=4\
>AT+CFIN=?\
>Certificates must be updated via LTE link terminal to modem\
>SEC TAG FOR AWS IS __123456__

| AWS Cloud files | COPIED | NRF Modem | 
| :-------: | :--------: | :------: |
| AmazonRootCA1(1) | TO | ca-cert |
| 4e52-private.pem | TO | private-key |
| 4752-certificate.pem | TO | client-cert |

__7. Update the overlay-aws.conf file__

`````
CONFIG_AWS_FOTA=y
CONFIG_AWS_IOT_SEC_TAG=123456
CONFIG_AWS_IOT_BROKER_HOST_NAME="a30xxxxxxx-ats.iot.us-east-2.amazonaws.com"
CONFIG_AWS_FOTA_DOWNLOAD_SECURITY_TAG=123456
CONFIG_POSIX_MAX_FDS=24 
CONFIG_AWS_IOT_CLIENT_ID_STATIC="nrf9160_thing"
          OR
#CONFIG_AWS_IOT_CLIENT_ID_APP=y
`````

__8. To clone the project: use the following directory__

``````
"C:\Nordic_apps\myapps" 
and then create a new folder into it and in that folder download the repository from GitHub 
After that,
You have to add the build configurations to select the DK, in our case, we can use nrf9160dk_nrf9160_ns
``````
Configuration:

>prj.conf

Kconfig Fragments:

>overlay-aws.conf\

Extra-Cmake Argument

>-DOVERLAY_CONFIG=overlay-aws.conf

At the end 

>PRESS Build Configuration

__9. Code Explanation:__

* __overlay-aws.conf__

>CONFIG_AWS_IOT_BROKER_HOST_NAME=""
CONFIG_AWS_IOT_BROKER_HOST_NAME="a30695n0axlqin-ats.iot.us-east-2.amazonaws.com"

* __Prj.conf__
>#CONFIG_NRF_CLOUD_MQTT=y\
>#CONFIG_CLOUD_CLIENT_ID_USE_CUSTOM=y\
>#CONFIG_NRF_CLOUD_SEC_TAG=16842753

* __src/cloud-certs/client-cert.pem__
* __src/cloud-certs/private-key.pem__
  
  Copy paste the files from the AWS cloud as described point # 6 above

* __src/cloud/cloud_codec/aws_iot/json_protocol_names_aws_iot.h__

>#define TLV493D_DATA           "tlv493d"

* __src/cloud/cloud_codec/cloud_codec.h__
  
  >	-char * temperature;\
	+double temperature;

* __src/modules/sensor_module.c__

>sensor_module_event->data.sensors.temperature = dummyDouble ;

__10. AWS IoT Core Setting Images :__

* [Things](thing1.jpg)
* [Policy(A)](policy1.jpg)
* [Policy(B)](policy2.jpg) 
* [Certificates](certificates.jpg)
  >* Go to settings and check policy and certificates both are active
  >* Download the certificates
  >* [As Mentioned above](#6-certificate)
* [Attach Certicates with Thing](attach_thing.jpg)
* [Attach Certicates with Policy](attach_policy.jpg)
* [MQTT End Point (AWS)](endpoint1.jpg)
* [MQTT End Point (FW)](endpoint2.jpg)
* [Topic Subscribe /MQTT Test (AWS Wild Card)](Wildcard_topic_aws.jpg)
* [Copy Certificates(Downloaded from AWS)](Copy_Certificatess.jpg)
* [Load AT Client FW](AT_Client.jpg)
* [Load Certificates to Modem](Modem_cert.jpg)
* [Build Conf Gateway FW(A)](Build_Conf1.jpg)
* [Build Conf Gateway FW(B)](Build_Conf2.jpg)
  >Open COM3(Terminal Output) & COM4(Commands)
* [MQTT Test Terminal (Data from NRFDK)](Mqtt_Test_out.jpg)
* [MQTT.fx Broker Setting](mqtt_broker_setting.jpg)

__11. AWS DynamoDB & Lambda Setting Images :__

* [Rule(A)](rule1.jpg)
* [Rule(B)](rule2.jpg)
* [Rule Lambda Fn(C)](rule3.jpg)
* [Rule Lambda Fn(D)](rule4.jpg)
* [Lambda Fn Conf(A)](Lambda1.jpg)
* [Lambda Fn Conf(B)](Lambda2.jpg)
* [Lambda Fn Conf(C)](Lambda3.jpg)
* [Table](table1.jpg)
* [Table name Reference](table2.jpg)
* [IAM Access Error](iamaccess1.jpg)
* [IAM Access Role (A)](iamaccess2.jpg)
* [IAM Access Role (B)](iamaccess3.jpg)
* [IAM Access Permission (A)](permission1.jpg)
* [IAM Access Permission (B)](permission2.jpg)
* [IAM Access Permission (C)](permission3.jpg)
* [IAM Access Permission (D)](permission4.jpg)
* [Lambda Fn Test](lamdatest.jpg)
* [DynamoDB Table Explore Item](dynamodb_explore_item.jpg)
* [Data Publish From Mqtt.fx](mqtt_pub.jpg)
* [Updated DynamoDB Table](mqtt_pub_data.jpg)

__12. FOTA AWS Setting Images:__

* [Bucket ](bucket1.jpg)
* [Bucket app_data.bin](bucket2.jpg)
* [Bucket Job Doc](bucket3.jpg)
* [Bucket Files Added](bucket4.jpg)
* [Bucket name Job Doc](bucket5.jpg)
* [Job (A)](job1.jpg)
* [Job (B)](job2.jpg)
* [Job (C)](job3.jpg)
* [Job (D)](job4.jpg)

__13. FOTA Started:__

* [FOTA Started ](fota1.jpg)
* [FOTA Done! ](fota2.jpg)
* [FW Updated ](fota3.jpg)
  

__14.AWS IoT Core And DynamoDB Video Links:__

[AWS IoT Core](https://youtu.be/adKuyckikuw)

[DynamoDB & Lambda Function](https://youtu.be/0RcVwTKSbSA)

__15.Lambda Fn Code:__

``````
import boto3

def lambda_handler(event, context):
    # Existing code to extract other parameters
    timestamp = str(event['state']['reported']['roam']['ts'])
    band = event['state']['reported']['roam']['v']['band']
    network = event['state']['reported']['roam']['v']['nw']
    rsrp = event['state']['reported']['roam']['v']['rsrp']
    area = event['state']['reported']['roam']['v']['area']
    mccmnc = event['state']['reported']['roam']['v']['mccmnc']
    cell = event['state']['reported']['roam']['v']['cell']
    ip = event['state']['reported']['roam']['v']['ip']
   # tlv493d = event['state']['reported']['roam']['v']['tlv493d']

    # Split the tlv493d values
    tlv493d_values = event['state']['reported']['roam']['v']['tlv493d'].split()
   # tlv493d_values_converted = [int(value, 16) for value in tlv493d_values]
    
       # Specific names for each of the 32 tlv493d values
    tlv493d_column_names = [
        'N1','N2', 'N3', 'N4','N5', 'N6', 'N7', 'N8','N9', 'N10', 'N11', 'N12','N13','N14','N15','N16','N17','N18','N19','N20',
        'N21','N22','N23','N24','N25','N26','N27','N28','N29','N30','N31','N32'
    ]


    # Check if there are 32 values, otherwise handle the error or adjust as needed
    if len(tlv493d_values) != 32:
        print("Error: tlv493d does not have 32 values.")
        return {
            'statusCode': 500,
            'body': 'Error: tlv493d does not have 32 values.'
        }

    # Preparing the item for DynamoDB
    item = {
        'timestamp': {'S': timestamp},
        'band': {'N': str(band)},
        'network': {'S': network},
        'rsrp': {'N': str(rsrp)},
        'area': {'N': str(area)},
        'mccmnc': {'N': str(mccmnc)},
        'cell': {'N': str(cell)},
        'ip': {'S': ip}
        #'tlv493d': {'S': tlv493d}
    }

    # Add tlv493d values to the item
    for i, value in enumerate(tlv493d_values):
        column_name = tlv493d_column_names[i]
        item[column_name] = {'S': value}

    # DynamoDB client
    client = boto3.client('dynamodb')

    # Storing the item in DynamoDB
    response = client.put_item(
        TableName='nrf9160_table',
        Item=item
    )

    return {
        'statusCode': 200,
        'body': 'Data processed successfully'
    }



``````
__16.MQTT Json Packet:__

``````
{
  "state": {
    "reported": {
      "roam": {
        "v": {
          "band": 12,
          "nw": "LTE-M",
          "rsrp": -76,
          "area": 36867,
          "mccmnc": 310410,
          "cell": 84690705,
          "ip": "10.165.223.207",
          "tlv493d": "B0 FC B2 B3 01 01 01 A8 24 75 25 84 59 99 99 FF FF AA AA AA A9 41 3E 5E DC 01 01 86 FF FF FF FF "
        },
        "ts": 1709134814587
      }
    }
  }
}
``````

[Dynamodb Reference # 1 ](https://docs.aws.amazon.com/iot/latest/developerguide/iot-ddb-rule.html#iot-ddb-rule-ddb-table)

[Dynamodb Reference # 2 ](https://docs.aws.amazon.com/pdfs/iot/latest/developerguide/iot-dg.pdf#iot-sql-reference)

[AWS IoT Reference # 1](https://devzone.nordicsemi.com/f/nordic-q-a/107117/aws-view-data-nrf9160-asset-tracker-v2)

[AWS IoT Reference # 2](https://devzone.nordicsemi.com/f/nordic-q-a/83129/asset-tracker-v2-connecting-problem-to-aws-iot/345057)

[AWS IoT Reference # 3](https://how2electronics.com/connecting-esp32-to-amazon-aws-iot-core-using-mqtt/#google_vignette)

[AWS IoT Topic Reference # 1](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/applications/asset_tracker_v2/doc/cloud_wrapper.html#id1)

[AWS IoT Job](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/libraries/networking/aws_iot.html#aws-setup-and-configuration)

[AWS FOTA Job](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/libraries/networking/aws_fota.html#creating-a-fota-job)






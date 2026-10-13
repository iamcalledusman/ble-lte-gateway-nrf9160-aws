nRF9160 BLE-to-AWS IoT LTE Gateway
An LTE gateway firmware for the Nordic nRF9160 that receives sensor packets from a Bluetooth central (nRF52833) over UART, adds gateway metadata (LTE signal level, sequence counter, reception time, checksum), and publishes them to AWS IoT Core over MQTT. An IoT rule and a Lambda function then store each packet in Amazon DynamoDB. Firmware updates are delivered over the air with AWS FOTA.
It is built on Nordic's open-source Asset Tracker v2 (nRF Connect SDK v2.4.0), extended with a custom UART module, a packet builder, and an AWS IoT Core integration.
Overview
```
 BLE peripherals            nRF52833 central               nRF9160 gateway
 (sensor nodes)  --BLE-->  adds its own parameters  --UART1-->  adds gateway metadata
                                                                      |
                                                                      | LTE (MQTT, TLS)
                                                                      v
   Amazon DynamoDB  <--  Lambda function  <--  IoT rule  <--  AWS IoT Core
         ^
         |                      S3 bucket + IoT job  --FOTA-->  nRF9160 gateway
```
Features
LTE-M / NB-IoT connectivity with PSM power saving (inherited from Asset Tracker v2)
MQTT over TLS to AWS IoT Core using device certificates stored in the modem
UART1 receiver for packets from the central, framed as `($ ... &)` and terminated with a NUL byte
Event-driven design with the Application Event Manager: a `uart_module_event` triggers a data sample and cloud upload
32-byte packet with gateway-added fields and a CRC-based checksum
AWS pipeline: IoT rule, Lambda function, DynamoDB table (one column per packet byte, N1 to N32)
FOTA updates from an S3 bucket through AWS IoT jobs
Runs on the nRF9160 DK and on a custom board
Hardware and tools
Item	Details
Gateway	nRF9160 DK (`nrf9160dk_nrf9160_ns`) or custom board
Central	nRF52833, forwards sensor data to the gateway over UART1
Cloud	AWS account (IoT Core, Lambda, DynamoDB, S3, IAM)
SDK	nRF Connect SDK v2.4.0
IDE	VS Code with the nRF Connect extension
Tools	Docklight (serial terminal), LTE Link Monitor, MQTT.fx (optional)
Demo
Gateway start-up log (UART0, Docklight). The gateway registers on the LTE network, connects to the cloud, and sends its first data.
![Gateway start-up log](docs/images/cloud_connectivity.png)
Packet from the central (UART1, Docklight). A test frame sent to the gateway.
![Packet from the central](docs/images/central_packet.png)
Frame decoding check. The frame in the sequence editor, used to verify that the cloud receives the correct data.
![Command from the central](docs/images/command_from_central.png)
Packet format
The gateway publishes a 32-byte packet as 32 space-separated hex values in the `tlv493d` field of the modem-dynamic (`roam`) JSON object.
Byte(s)	Content	Source
0 to 21	Sensor data and central parameters	Central (UART)
9 to 15	Any `0x00` is replaced with `0x01` (a NUL cannot be sent)	Gateway
20 (low nibble)	LTE RSRP level, 0 to 15	Gateway (modem)
22	High nibble: gateway TX sequence (0 to 15, wraps). Low nibble: `0xE` (battery field)	Gateway
23 to 25	Reception time in seconds since midnight (UTC), little-endian	Gateway
26	Flag byte: bit 0 always set, bit n+1 set if byte 9+n was `0x00` and was replaced	Gateway
27	Checksum: low 8 bits of CRC-16/CCITT (polynomial `0x1021`) over bytes 0 to 26	Gateway
28 to 31	`0xFF`, reserved for future use	Gateway
Example MQTT message:
```json
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
          "tlv493d": "B0 FC B2 B3 01 01 01 A8 24 75 25 84 59 99 99 FF FF AA AA AA A9 41 3E 5E DC 01 01 86 FF FF FF FF"
        },
        "ts": 1709134814587
      }
    }
  }
}
```
Getting started
1. Clone the repository
```bash
git clone https://github.com/<your-username>/<repo-name>.git
```
Keep the path short on Windows (for example `C:\Nordic_apps\myapps\<repo-name>`) to avoid build errors from long paths.
2. Set up AWS IoT Core
Create a Thing, a policy, and a certificate, then attach them to each other. Download the certificate files.
<details>
<summary><b>AWS IoT Core setup screenshots</b></summary>
Create the Thing
![Create the Thing](docs/images/thing1.jpg)
Create the policy (A)
![Create the policy (A)](docs/images/policy1.jpg)
Create the policy (B)
![Create the policy (B)](docs/images/policy2.jpg)
Create certificates - make sure the certificate and policy are both active, then download the files
![Create certificates - make sure the certificate and policy are both active, then download the files](docs/images/certificates.jpg)
Attach the certificate to the Thing
![Attach the certificate to the Thing](docs/images/attach_thing.jpg)
Attach the certificate to the policy
![Attach the certificate to the policy](docs/images/attach_policy.jpg)
MQTT endpoint (AWS console)
![MQTT endpoint (AWS console)](docs/images/endpoint1.jpg)
MQTT endpoint (firmware configuration)
![MQTT endpoint (firmware configuration)](docs/images/endpoint2.jpg)
Subscribe to a wildcard topic in the MQTT test client
![Subscribe to a wildcard topic in the MQTT test client](docs/images/Wildcard_topic_aws.jpg)
</details>
3. Provision the certificates into the modem
The certificates are written to the modem with security tag `123456`. Do not commit them to the repository.
Flash the AT Client sample to the DK.
Put the modem offline: `AT+CFUN=4`
In LTE Link Monitor, open the certificate manager, select security tag `123456`, and paste the three files:
AWS file	Paste into (modem)
`AmazonRootCA1.pem`	CA certificate
`<id>-certificate.pem.crt`	Client certificate
`<id>-private.pem.key`	Private key
<details>
<summary><b>Certificate provisioning screenshots</b></summary>
Downloaded AWS certificate files
![Downloaded AWS certificate files](docs/images/Copy_Certificatess.jpg)
Load the AT Client firmware on the DK
![Load the AT Client firmware on the DK](docs/images/AT_Client.jpg)
Load the certificates into the modem (LTE Link Monitor)
![Load the certificates into the modem (LTE Link Monitor)](docs/images/Modem_cert.jpg)
</details>
4. Configure the firmware
Edit `overlay-aws.conf` and set your own AWS IoT endpoint (found in the AWS IoT console under Settings):
```ini
CONFIG_AWS_IOT=y
CONFIG_AWS_FOTA=y
CONFIG_AWS_IOT_SEC_TAG=123456
CONFIG_AWS_IOT_BROKER_HOST_NAME="<your-endpoint>-ats.iot.<region>.amazonaws.com"
CONFIG_AWS_FOTA_DOWNLOAD_SECURITY_TAG=123456
CONFIG_POSIX_MAX_FDS=24
CONFIG_AWS_IOT_CLIENT_ID_STATIC="nrf9160_thing"
# or let the application generate the client ID:
# CONFIG_AWS_IOT_CLIENT_ID_APP=y
```
5. Build and flash
In the nRF Connect extension, add a build configuration with:
Setting	Value
Board	`nrf9160dk_nrf9160_ns`
Configuration	`prj.conf`
Kconfig fragment	`overlay-aws.conf`
Extra CMake argument	`-DOVERLAY_CONFIG=overlay-aws.conf`
Then press Build Configuration, build, and flash. Open COM3 for the terminal output and COM4 for commands to UART1.
<details>
<summary><b>Build configuration screenshots</b></summary>
Build configuration for the gateway firmware (A)
![Build configuration for the gateway firmware (A)](docs/images/Build_Conf1.jpg)
Build configuration for the gateway firmware (B) - open COM3 for terminal output and COM4 for commands
![Build configuration for the gateway firmware (B) - open COM3 for terminal output and COM4 for commands](docs/images/Build_Conf2.jpg)
</details>
6. Verify in the MQTT test client
Subscribe to a wildcard topic in the AWS IoT MQTT test client. The gateway's messages should appear after it connects.
<details>
<summary><b>MQTT test screenshots</b></summary>
MQTT test client showing data published by the nRF9160 DK
![MQTT test client showing data published by the nRF9160 DK](docs/images/Mqtt_Test_out.jpg)
MQTT.fx broker settings
![MQTT.fx broker settings](docs/images/mqtt_broker_setting.jpg)
</details>
Store data in DynamoDB
An IoT rule forwards each device-shadow update to a Lambda function, which splits the 32 hex values and writes one DynamoDB item per packet, with columns `N1` to `N32` plus band, network, RSRP, area, cell, and IP. The function code is in `lambda/lambda_function.py`.
<details>
<summary><b>IoT rule</b></summary>
IoT rule (A)
![IoT rule (A)](docs/images/rule1.jpg)
IoT rule (B)
![IoT rule (B)](docs/images/rule2.jpg)
Rule action - Lambda function (C)
![Rule action - Lambda function (C)](docs/images/rule3.jpg)
Rule action - Lambda function (D)
![Rule action - Lambda function (D)](docs/images/rule4.jpg)
</details>
<details>
<summary><b>Lambda function</b></summary>
Lambda function configuration (A)
![Lambda function configuration (A)](docs/images/Lambda1.jpg)
Lambda function configuration (B)
![Lambda function configuration (B)](docs/images/Lambda2.jpg)
Lambda function configuration (C)
![Lambda function configuration (C)](docs/images/Lambda3.jpg)
Lambda function test
![Lambda function test](docs/images/lamdatest.jpg)
</details>
<details>
<summary><b>DynamoDB table</b></summary>
DynamoDB table
![DynamoDB table](docs/images/table1.jpg)
Table name reference
![Table name reference](docs/images/table2.jpg)
Explore table items
![Explore table items](docs/images/dynamodb_explore_item.jpg)
Publish test data from MQTT.fx
![Publish test data from MQTT.fx](docs/images/mqtt_pub.jpg)
Updated DynamoDB table after publishing
![Updated DynamoDB table after publishing](docs/images/mqtt_pub_data.jpg)
</details>
Troubleshooting: IAM access
If the Lambda test fails with `AccessDeniedException` on `PutItem`, the function's execution role needs permission to write to the table.
<details>
<summary><b>IAM screenshots</b></summary>
IAM access error (AccessDeniedException on PutItem)
![IAM access error (AccessDeniedException on PutItem)](docs/images/iamaccess1.jpg)
IAM access role (A)
![IAM access role (A)](docs/images/iamaccess2.jpg)
IAM access role (B)
![IAM access role (B)](docs/images/iamaccess3.jpg)
IAM permission (A)
![IAM permission (A)](docs/images/permission1.jpg)
IAM permission (B)
![IAM permission (B)](docs/images/permission2.jpg)
IAM permission (C)
![IAM permission (C)](docs/images/permission3.jpg)
IAM permission (D)
![IAM permission (D)](docs/images/permission4.jpg)
</details>
Firmware over the air (FOTA)
Upload the application image (`app_update.bin`) and a job document to an S3 bucket.
Create an AWS IoT job that targets the Thing.
The gateway downloads the image, verifies it, and reboots into the new firmware (MCUboot).
<details>
<summary><b>S3 bucket</b></summary>
S3 bucket
![S3 bucket](docs/images/bucket1.jpg)
Upload app_update.bin to the bucket
![Upload app_update.bin to the bucket](docs/images/bucket2.jpg)
Job document
![Job document](docs/images/bucket3.jpg)
Files added to the bucket
![Files added to the bucket](docs/images/bucket4.jpg)
Job document name / object reference
![Job document name / object reference](docs/images/bucket5.jpg)
</details>
<details>
<summary><b>Create the job</b></summary>
Create the FOTA job (A)
![Create the FOTA job (A)](docs/images/job1.jpg)
Create the FOTA job (B)
![Create the FOTA job (B)](docs/images/job2.jpg)
Create the FOTA job (C)
![Create the FOTA job (C)](docs/images/job3.jpg)
Create the FOTA job (D)
![Create the FOTA job (D)](docs/images/job4.jpg)
</details>
<details>
<summary><b>FOTA in action</b></summary>
FOTA started
![FOTA started](docs/images/fota1.jpg)
FOTA done
![FOTA done](docs/images/fota2.jpg)
Firmware updated
![Firmware updated](docs/images/fota3.jpg)
</details>
What was changed from Asset Tracker v2
File	Change
`src/uart_1_module.c`, `src/uart_1_event.c/.h`	New: UART event type and listener
`src/main.c`	UART1 interrupt callback, frame extraction (`($` ... `&)`), event submission
`src/modules/data_module.c`	Handles UART events and builds the 32-byte packet (RSRP, sequence, time, flag byte, CRC)
`src/events/modem_module_event.h`, `src/cloud/cloud_codec/cloud_codec.h`	Added the `tlv493d[150]` field
`src/cloud/cloud_codec/json_common.c`	Encodes the packet into the cloud JSON
`src/cloud/cloud_codec/aws_iot/json_protocol_names_aws_iot.h`	Added the `tlv493d` key name
`src/modules/sensor_module.c`	Temperature changed to a numeric (double) value
`overlay-aws.conf`, `prj.conf`	AWS IoT and FOTA configuration, nRF Cloud options disabled
`boards/nrf9160dk_nrf9160_ns.overlay`	UART1 enabled at 115200 baud
Known limitations and ideas
Fixed-size buffers in the UART parser (`Data_arr[50]`, 8-bit index) need bounds checks, and the frame search should handle a missing `($` or `&)`
The JSON key is still named `tlv493d` from the original sensor prototype
The battery nibble in byte 22 is a fixed value (`0xE`)
Several globals are shared between `main.c` and `data_module.c`
Add unit tests for the frame parser, flag byte, and checksum
Videos and references
AWS IoT Core walkthrough
DynamoDB and Lambda walkthrough
AWS IoT rule to DynamoDB
AWS IoT SQL reference
Asset Tracker v2 cloud wrapper
nRF Connect SDK: AWS IoT library setup
nRF Connect SDK: creating an AWS FOTA job
License
Based on Asset Tracker v2 from Nordic Semiconductor's nRF Connect SDK. Original code is (c) Nordic Semiconductor ASA, and this project keeps the same license, LicenseRef-Nordic-5-Clause. See LICENSE. This license allows use only with Nordic Semiconductor integrated circuits.
Author
Muhammad Usman
GitHub: @iamcalledusman

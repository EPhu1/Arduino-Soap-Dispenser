# Arduino-Soap-Dispenser

Libaries Used:
-HX711_ADC 
-EEPROM
-WiFiEsp


How to connect to AWS:
https://linuxize.com/post/how-to-install-flask-on-ubuntu-18-04/
ssh -i "keypair.pem" ubuntu@ec2-13-58-148-61.us-east-2.compute.amazonaws.com
cd my_flask_app
source venv/bin/activate
cd ..
export FLASK_APP=server.py
python3 -m flask run --host=0.0.0.0


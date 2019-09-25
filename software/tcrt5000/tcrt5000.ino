#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED display width, in pixels
#define DISPLAY_WIDTH 128 
// OLED display height, in pixels
#define DISPLAY_HEIGHT 64
// I2C Address for the display
#define DISPLAY_I2C_ADDRESS		0x3c

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// Reset pin # (or -1 if sharing Arduino reset pin)
#define OLED_RESET     4


Adafruit_SSD1306 display( DISPLAY_WIDTH, DISPLAY_HEIGHT, &Wire, OLED_RESET );


// Analog input for TCRT5000
#define SENSOR_ANALOG_INPUT		 A2

// Last measured sensor value
uint16_t sensor_value = 0;

// Average measurements to reduce the noise
#define AVERAGE_LENGTH    8
uint16_t average_buffer[ AVERAGE_LENGTH ];
uint8_t average_index = 0;


void setup()
{
	Serial.begin(9600);

	if( display.begin( SSD1306_SWITCHCAPVCC, DISPLAY_I2C_ADDRESS ) == false )
	{
		Serial.println( F( "SSD1306 allocation failed" ) );
		while( true )
		{
			// block here
		}
	}

	// Display static elements
	// "Distance" header
	// and the rectangle for the fill bar.
	display.clearDisplay();
	display.setTextSize( 2 );
	display.setTextColor( WHITE );
	display.setCursor( 0, 0 );
	display.print( F( "Distance" ) );
	display.drawRect( 0, 50, DISPLAY_WIDTH, 10, WHITE );

	// print the distance (0) by default
	// this step is needed, because subsequent calls to print_distance()
	// erase previous values by overwriting them with black ink
	print_distance();
}

void loop() 
{
	measure_distance();
	print_distance();
	draw_distance();
}

void measure_distance( void )
{
	uint32_t average = 0;

	average_buffer[ average_index ] = analogRead( SENSOR_ANALOG_INPUT );
	++ average_index;
	if( average_index >= AVERAGE_LENGTH )
	{
		average_index = 0;
	}
  
	for( uint8_t i = 0; i < AVERAGE_LENGTH; ++ i )
	{
		average += average_buffer[ i ];
	}
	average /= AVERAGE_LENGTH;
	sensor_value = (uint16_t) average;
	Serial.println( sensor_value );
}

void print_distance( void )
{
	static uint16_t previous_sensor_value = 0;

	if( previous_sensor_value == sensor_value )
	{
		// don't do expensive graphic operations
		return;
	}
	// erase previous value
	display.setTextColor( BLACK );
	display.setCursor( 0, 20 );
	display.print( previous_sensor_value );
	// write new value
	display.setTextColor( WHITE );
	display.setCursor( 0, 20 );
	display.print( sensor_value );
	display.display();
	previous_sensor_value = sensor_value;
}

void draw_distance( void )
{
	static uint16_t previous_bar_width;
	uint16_t bar_width = 126.0 * ( sensor_value / 1024.0 );
	
	if( bar_width == previous_bar_width )
	{
  		// don't do expensive graphic operations
		return;
	}

	if( bar_width > previous_bar_width )
	{
 		// extend the bar
		display.fillRect( previous_bar_width, 51, bar_width - previous_bar_width, 8, WHITE );
	}
	else
	{
		// cut the bar
		display.fillRect( bar_width, 51, previous_bar_width - bar_width, 8, BLACK );
	}
  
	display.display();
	previous_bar_width = bar_width;
}

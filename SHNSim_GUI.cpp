#include <iostream>
#include <gtk/gtk.h>
#include <cairo.h>
#include <math.h>
#include <string>
#include <sstream>
#include <gdk/gdkkeysyms.h>
#include <vector>
#include <utility>

using namespace std;

// define structure to hold windows used for project
struct
{
	GtkWidget* DrawingWindow;
	GtkWidget* SimParamWindow;
	GtkWidget* DiagnosticsWindow;
	
} WINDOWS;

// define structure to hold window dimensions
struct
{
	int WIDTH;
	int HEIGHT;
} SCREEN;

// define structure that holds a set of coordinates (x,y)
struct coord
{
	double x;
	double y;
};

// define structure that represents the list of nodes (hexagons) in the system
// as well as other parameters needed to run the simulation
struct
{
	int count, selectedTile, highlightedSide;
	double sideLength, screenWidth, screenHeight, mouseX, mouseY;
 	vector<pair<int, int>> neighbors[1000];
 	vector<vector<double>> coords;
 	vector<int> state;	// 0 = healthy, 1 = congested, 2 = alt congested, 3 = down
 	vector<int> path;
 	
	// stage 2 parameters
	int bsLen = 5;
	int antNum = 3;
	int transNum = 100;
	double transDist = 0.002;
	int dRateMax = 10;
	int uePerAnt = 10;
	int simLen = 28800;
	int simNum = 1;
	int simStartNum = 0;
	string simName = "default name";
	int bufSize = 10;
	
} glob;

// define a struct to hold references to entry boxes (used to pass
// entry from the text boxes throughout the entire program)
struct
{
	GtkWidget *baseStationSide, *antennaNumber, *transceiverNum, *transceiverDist, *maxDataRate, *userEquipPerAntenna;
	GtkWidget *simulationLength, *simulationNumber, *simulationStart, *simulationSaveName, *bufferSize; 
	
} entryBoxes;

// window setup function prototypes
void setUpDrawingWindow();
void setUpSimParamWindow();
void setUpDiagnosticsWindow();

// navigation function prototypes - used to change windows
void goToSimParams();
void runSim();
void backToDrawingStage();

// functions used in drawing window
static void getScreenHeight();
static void drawHex(cairo_t *);
static void button_clicked(GtkWidget* widget, gpointer data);
static gboolean mouse_moved(GtkWidget *widget, GdkEvent *event, gpointer user_data);
static gboolean mouse_clicked(GtkWidget *widget, GdkEventButton *event, gpointer user_data);
static gboolean on_draw_event(GtkWidget *widget, cairo_t *cr, gpointer user_data);
static float distance(double x1, double y1, double x2, double y2);
static int side(double x1, double y1, double x2, double y2);
static bool deletionValid(int tile);
static void getNeighbors();

// Utility function(s)
void getDimensions();

// function to add parameters to param struct; used to pass params to run the simulation
void addParams();

int main(int argc, char** argv)
{
	// initialize gtk	
	gtk_init(&argc, &argv);
	
	// get screen dimensions
	getDimensions();
	
	// set up all windows
	setUpDrawingWindow();
	setUpSimParamWindow();
	
	// initialize the system by making the first window visible
	gtk_widget_show_all(WINDOWS.DrawingWindow);
	
	gtk_main();
	return 0;
}

void setUpDrawingWindow()
{
	GtkWidget *window, *darea, *button, *fixed;

	glob.count = 0;
	glob.selectedTile = 0;
	glob.highlightedSide = 0;
	
	glob.screenHeight = (SCREEN.HEIGHT - 58);
	glob.screenWidth = (SCREEN.WIDTH - 80);
	glob.sideLength = glob.screenHeight * 0.25;
	
  	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	WINDOWS.DrawingWindow = window;

  	darea = gtk_drawing_area_new();
  	button = gtk_button_new_with_label("Next Page");
	fixed = gtk_fixed_new();
  
	g_signal_connect(button, "clicked", G_CALLBACK(button_clicked), NULL);
  	g_signal_connect(G_OBJECT(darea), "draw", G_CALLBACK(on_draw_event), NULL); 
  	g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);  
  	g_signal_connect(window, "button-press-event", G_CALLBACK(mouse_clicked), NULL);
	g_signal_connect (window, "motion-notify-event", G_CALLBACK (mouse_moved), NULL);
 	
 	gtk_widget_add_events(window, GDK_BUTTON_PRESS_MASK);
 	gtk_widget_set_events(window, GDK_POINTER_MOTION_MASK);
  	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_window_set_default_size(GTK_WINDOW(window), glob.screenWidth, glob.screenHeight); 
  
  	gtk_container_add(GTK_CONTAINER(window), fixed);
  
  	gtk_fixed_put(GTK_FIXED(fixed), button, glob.screenWidth * 0.95, glob.screenHeight * 0.95);
  	gtk_widget_set_size_request(button, glob.screenWidth * 0.05, glob.screenHeight * 0.05);
  	gtk_fixed_put(GTK_FIXED(fixed), darea, 0, 0);
  	gtk_widget_set_size_request(darea, glob.screenWidth * 0.95, glob.screenHeight * 0.95);
  
  	gtk_window_set_title(GTK_WINDOW(window), "5G Simulator");

  	GdkColor black = {0, 0x0000, 0x0000, 0x0000};
  	gtk_widget_modify_bg(window, GTK_STATE_NORMAL, &black);
}

void setUpSimParamWindow()
{
	// create stage 2 window and configure window properties
	GtkWidget* s2Window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(s2Window), "Self-Healing Simulator");
	gtk_window_set_default_size(GTK_WINDOW(s2Window), SCREEN.WIDTH - 60, SCREEN.HEIGHT - 58);
	g_signal_connect(s2Window, "delete-event", G_CALLBACK(gtk_main_quit), NULL);
	
	// copy window into struct objcet for global use across functions
	WINDOWS.SimParamWindow = s2Window;
	
	// create input labels and text boxes from stage 2 of C# code
	GtkWidget *bsSide, *numAntenna, *numTransceivers, *distTransceivers, *maxDR, *uePerAntenna; // labels
	GtkWidget *bsSideTxt, *numAntennaTxt, *numTransceiversTxt, *distTransceiversTxt, *maxDRTxt, *uePerAntennaTxt; // textbox
	
	// create input labels and text boxes from stage 3 of C# code
	GtkWidget *simLength, *simNum, *simStart, *simSaveName, *bufSize; // labels
	GtkWidget *simLengthTxt, *simNumTxt, *simStartTxt, *simSaveNameTxt, *bufSizeTxt; // textboxes
	
	// create back button and run simulation button
	GtkWidget *backToS1Btn, *runSimBtn;
	
	// instantiate input labels, textboxes, and buttons
	bsSide = gtk_label_new("BS Side Length (5 < n < 20)");
	bsSideTxt = gtk_entry_new();
	numAntenna = gtk_label_new("Number of Antenna (1 < n < 6)");
	numAntennaTxt = gtk_entry_new();
	numTransceivers = gtk_label_new("Number of Transceivers (80 < n < 200)");
	numTransceiversTxt = gtk_entry_new();
	distTransceivers = gtk_label_new("Distance between Transceivers (0.001 < n < 0.00865)");
	distTransceiversTxt = gtk_entry_new();
	maxDR = gtk_label_new("Enter Max Data Rate for UE (5 < n < 20)");
	maxDRTxt = gtk_entry_new();
	uePerAntenna = gtk_label_new("Enter UEs per Antenna [normal BS] (1 < n < 40)");
	uePerAntennaTxt = gtk_entry_new();
	
	simLength = gtk_label_new("Length of Simulation (seconds)");
	simLengthTxt = gtk_entry_new();
	simNum = gtk_label_new("Number of Simulations");
	simNumTxt = gtk_entry_new();
	simStart = gtk_label_new("Simulation Starting Number");
	simStartTxt = gtk_entry_new();
	simSaveName = gtk_label_new("Simulation Save Name");
	simSaveNameTxt = gtk_entry_new();
	bufSize = gtk_label_new("Buffer size");
	bufSizeTxt = gtk_entry_new();
	
	backToS1Btn = gtk_button_new_with_label("Back");
	runSimBtn = gtk_button_new_with_label("Run Simulation");
	
	// connect button signals
	g_signal_connect(backToS1Btn, "clicked", G_CALLBACK(backToDrawingStage), NULL);
	g_signal_connect(runSimBtn, "clicked", G_CALLBACK(runSim), NULL);
	
	// instantiate 2 columns to hold simulation inputs
	GtkWidget* bsInputs = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	GtkWidget* simInputs = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	GtkWidget* hBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_set_homogeneous(GTK_BOX(hBox), TRUE);
	GtkWidget* mainBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_box_set_homogeneous(GTK_BOX(hBox), FALSE);
	
	// pack bs input labels and textboxes into bs inputs container
	gtk_box_pack_start(GTK_BOX(bsInputs), bsSide, 0, 0, 15);
	gtk_box_pack_start(GTK_BOX(bsInputs), bsSideTxt, 0, 0, 0);
	gtk_box_pack_start(GTK_BOX(bsInputs), numAntenna, 0, 0, 15);
	gtk_box_pack_start(GTK_BOX(bsInputs), numAntennaTxt, 0, 0, 0);
	gtk_box_pack_start(GTK_BOX(bsInputs), numTransceivers, 0, 0, 15);
	gtk_box_pack_start(GTK_BOX(bsInputs), numTransceiversTxt, 0, 0, 0);
	gtk_box_pack_start(GTK_BOX(bsInputs), distTransceivers, 0, 0, 15);
	gtk_box_pack_start(GTK_BOX(bsInputs), distTransceiversTxt, 0, 0, 0);
	gtk_box_pack_start(GTK_BOX(bsInputs), maxDR, 0, 0, 15);
	gtk_box_pack_start(GTK_BOX(bsInputs), maxDRTxt, 0, 0, 0);
	gtk_box_pack_start(GTK_BOX(bsInputs), uePerAntenna, 0, 0, 15);
	gtk_box_pack_start(GTK_BOX(bsInputs), uePerAntennaTxt, 0, 0, 0);
	gtk_box_pack_end(GTK_BOX(bsInputs), backToS1Btn, 0, 0, 30);
	
	// pack bs input labels and textboxes into sim inputs container
	gtk_box_pack_start(GTK_BOX(simInputs), simLength, 0, 0, 15);
	gtk_box_pack_start(GTK_BOX(simInputs), simLengthTxt, 0, 0, 0);
	gtk_box_pack_start(GTK_BOX(simInputs), simNum, 0, 0, 15);
	gtk_box_pack_start(GTK_BOX(simInputs), simNumTxt, 0, 0, 0);
	gtk_box_pack_start(GTK_BOX(simInputs), simStart, 0, 0, 15);
	gtk_box_pack_start(GTK_BOX(simInputs), simStartTxt, 0, 0, 0);
	gtk_box_pack_start(GTK_BOX(simInputs), simSaveName, 0, 0, 15);
	gtk_box_pack_start(GTK_BOX(simInputs), simSaveNameTxt, 0, 0, 0);
	gtk_box_pack_start(GTK_BOX(simInputs), bufSize, 0, 0, 15);
	gtk_box_pack_start(GTK_BOX(simInputs), bufSizeTxt, 0, 0, 0);
	gtk_box_pack_end(GTK_BOX(simInputs), runSimBtn, 0, 0, 30);
	
	// pack bs inputs and sim inputs into 2 column container
	gtk_box_pack_start(GTK_BOX(hBox), bsInputs, 1, 1, 10);
	gtk_box_pack_start(GTK_BOX(hBox), simInputs, 1, 1, 10);
	
	// create title label
	GtkWidget* title = gtk_label_new("Simulation Parameters");
	gtk_widget_set_name(title, "title");
	
	// add title and 2 column container to the window
	gtk_box_pack_start(GTK_BOX(mainBox), title, 0, 1, 10);
	gtk_box_pack_start(GTK_BOX(mainBox), hBox, 1, 1, 0);
	gtk_container_add(GTK_CONTAINER(s2Window), mainBox);
	
	// add entry boxes (textboxes) to entry box struct
	entryBoxes.baseStationSide = bsSideTxt;
	entryBoxes.antennaNumber = numAntennaTxt;
	entryBoxes.transceiverNum = numTransceiversTxt;
	entryBoxes.transceiverDist = distTransceiversTxt;
	entryBoxes.maxDataRate = maxDRTxt;
	entryBoxes.userEquipPerAntenna = uePerAntennaTxt;
	entryBoxes.simulationLength = simLengthTxt;
	entryBoxes.simulationNumber = simNumTxt;
	entryBoxes.simulationStart = simStartTxt;
	entryBoxes.simulationSaveName = simSaveNameTxt;
	entryBoxes.bufferSize = bufSizeTxt;
	
	// load color settings for the GUI from CSS file
	GtkCssProvider* guiProvider = gtk_css_provider_new();
	if(gtk_css_provider_load_from_path(guiProvider, "", NULL)) // SHNSim.css
	{
		// labels
		gtk_style_context_add_provider(gtk_widget_get_style_context(bsSide), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		gtk_style_context_add_provider(gtk_widget_get_style_context(numAntenna), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		gtk_style_context_add_provider(gtk_widget_get_style_context(numTransceivers), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		gtk_style_context_add_provider(gtk_widget_get_style_context(distTransceivers), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		gtk_style_context_add_provider(gtk_widget_get_style_context(maxDR), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		gtk_style_context_add_provider(gtk_widget_get_style_context(uePerAntenna), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		gtk_style_context_add_provider(gtk_widget_get_style_context(simLength), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		gtk_style_context_add_provider(gtk_widget_get_style_context(simNum), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		gtk_style_context_add_provider(gtk_widget_get_style_context(simStart), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		gtk_style_context_add_provider(gtk_widget_get_style_context(simSaveName), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		gtk_style_context_add_provider(gtk_widget_get_style_context(bufSize), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);

		// buttons		
		gtk_style_context_add_provider(gtk_widget_get_style_context(backToS1Btn), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		gtk_style_context_add_provider(gtk_widget_get_style_context(runSimBtn), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);		
		
		// entry
		gtk_style_context_add_provider(gtk_widget_get_style_context(bsSideTxt), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		gtk_style_context_add_provider(gtk_widget_get_style_context(numAntennaTxt), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		gtk_style_context_add_provider(gtk_widget_get_style_context(numTransceiversTxt), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		gtk_style_context_add_provider(gtk_widget_get_style_context(distTransceiversTxt), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		gtk_style_context_add_provider(gtk_widget_get_style_context(maxDRTxt), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		gtk_style_context_add_provider(gtk_widget_get_style_context(uePerAntennaTxt), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		gtk_style_context_add_provider(gtk_widget_get_style_context(simLengthTxt), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		gtk_style_context_add_provider(gtk_widget_get_style_context(simNumTxt), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		gtk_style_context_add_provider(gtk_widget_get_style_context(simStartTxt), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		gtk_style_context_add_provider(gtk_widget_get_style_context(simSaveNameTxt), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		gtk_style_context_add_provider(gtk_widget_get_style_context(bufSizeTxt), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);

		// title
		gtk_style_context_add_provider(gtk_widget_get_style_context(title), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
	}
}

void setUpDiagnosticsWindow()
{
	// TODO - Will contain information about the system during runtime.
}

void goToSimParams()
{
	gtk_widget_show_all(WINDOWS.SimParamWindow);
	gtk_widget_hide_on_delete(WINDOWS.DrawingWindow);
}

// NOTE: does NOT open another window after running simulation (yet)
void runSim()
{
	// call a function to add values from entry boxes to parameter struct
	addParams();
	
	//gtk_widget_show_all(WINDOWS.DiagnosticsWindow);
	gtk_widget_hide_on_delete(WINDOWS.SimParamWindow);
	
	cout << "running..." << endl;
	
}

void backToDrawingStage()
{
	gtk_widget_show_all(WINDOWS.DrawingWindow);
	gtk_widget_hide_on_delete(WINDOWS.SimParamWindow);
}

static void drawHex(cairo_t *cr)
{
	if (glob.count == 0)
	{
		vector<double> test;
		test.push_back(glob.screenWidth * 0.95 / 2.0);
		test.push_back(glob.screenHeight * 0.95 / 2.0);
		glob.coords.erase(glob.coords.begin(), glob.coords.end());
		glob.coords.push_back(test);

		glob.state.erase(glob.state.begin(), glob.state.end());
		glob.state.push_back((int)0);

		glob.path.erase(glob.path.begin(), glob.path.end());
		glob.path.push_back(7);
		glob.count = 1;
	}
  	cairo_set_source_rgb(cr, 1, 1, 1);
	cairo_line_to(cr, 0, 0);
	cairo_line_to(cr, 0, glob.screenHeight);
	cairo_line_to(cr, glob.screenWidth, glob.screenHeight);
	cairo_line_to(cr, glob.screenWidth, 0);
	cairo_line_to(cr, 0, 0);
	cairo_fill(cr);		

	double mousedY = glob.coords[glob.selectedTile][1] - glob.mouseY;
	double mousedX = glob.mouseX - glob.coords[glob.selectedTile][0];

	double mouseparam = mousedY / mousedX;
	double mouseslope = atan(mouseparam) * 180.0 / M_PI;
	double mousedist = sqrt(mousedY * mousedY + mousedX * mousedX);
	if (abs(mouseslope) >= 60)	
	{
	 	if (mousedY < 0)
	  	{
		  	glob.highlightedSide = 0; // Bottom
		}
		else
		{
		 	glob.highlightedSide = 3; // Top
		}
	}
	else
	{
	  	if (mousedY < 0)
	  	{
	    	if (mousedX > 0)
	    	{
		   	glob.highlightedSide = 1;	// Bottom Right
	    	}
	    	else
	    	{
	     		glob.highlightedSide = 5;	// Bottom Left
	    	}
	  	}
	  	else
	  	{
	    	if (mousedX > 0)
	    	{
		    	glob.highlightedSide = 2;	// Top Right
	    	}
		   else
		   {
		    	glob.highlightedSide = 4;	// Top Left
		   }
	  	}
	}
  	cairo_set_source_rgb(cr, 0, 1, 0);
  	cairo_set_line_width(cr, 2.0);
	for (int i = 0; i < glob.count; i++)	// Fill
	{
		if (i == glob.selectedTile)
		{
			cairo_set_source_rgb(cr, 0, 100.0/255.0, 0);
		}
		else
		{
			cairo_set_source_rgb(cr, 0, 200.0/255.0, 0);
		}
		cairo_line_to(cr, glob.coords[i][0] + glob.sideLength * sin(2 * M_PI / 6 * (0.5 + 5)), glob.coords[i][1] + glob.sideLength * cos(2 * M_PI / 6 * (0.5 + 5)));
		
		for (int j = 0; j <= 5; j++)
		{
			cairo_line_to(cr, glob.coords[i][0] + glob.sideLength * sin(2 * M_PI / 6 * (0.5 + j)), glob.coords[i][1] + glob.sideLength * cos(2 * M_PI / 6 * (0.5 + j)));	
		}
		cairo_fill(cr);	
	} 
	for (int i = 0; i < glob.count; i++)	// Border
	{
		cairo_line_to(cr, glob.coords[i][0] + glob.sideLength * sin(2 * M_PI / 6 * (0.5 + 5)), glob.coords[i][1] + glob.sideLength * cos(2 * M_PI / 6 * (0.5 + 5)));
		for (int k = 0; k <= 5; k++)
		{
			cairo_set_source_rgb(cr, 0, 0, 0);
      	cairo_set_line_width(cr, 2.0);
			if (i == glob.selectedTile && k == glob.highlightedSide)
			{
				cairo_set_source_rgb(cr, 1, 0, 0);
        		cairo_set_line_width(cr, 4.0);
			}
			cairo_line_to(cr, glob.coords[i][0] + glob.sideLength * sin(2 * M_PI / 6 * (0.5 + k)), glob.coords[i][1] + glob.sideLength * cos(2 * M_PI / 6 * (0.5 + k)));
			cairo_stroke(cr);	
			if (k < 5)
			{
				cairo_line_to(cr, glob.coords[i][0] + glob.sideLength * sin(2 * M_PI / 6 * (0.5 + k)), glob.coords[i][1] + glob.sideLength * cos(2 * M_PI / 6 * (0.5 + k)));
			}
		}
	} 
	cairo_line_to(cr, glob.coords[glob.selectedTile][0] + glob.sideLength * sin(2 * M_PI / 6 * (0.5 + 5)), glob.coords[glob.selectedTile][1] + glob.sideLength * cos(2 * M_PI / 6 * (0.5 + 5)));
	for (int k = 0; k <= 5; k++)
	{
		cairo_set_source_rgb(cr, 0, 0, 0);
    	cairo_set_line_width(cr, 2.0);
		if (k == glob.highlightedSide)
		{
			cairo_set_source_rgb(cr, 1, 0, 0);
      	cairo_set_line_width(cr, 4.0);
		}
		cairo_line_to(cr, glob.coords[glob.selectedTile][0] + glob.sideLength * sin(2 * M_PI / 6 * (0.5 + k)), glob.coords[glob.selectedTile][1] + glob.sideLength * cos(2 * M_PI / 6 * (0.5 + k)));
		cairo_stroke(cr);	
		if (k < 5)
		{
			cairo_line_to(cr, glob.coords[glob.selectedTile][0] + glob.sideLength * sin(2 * M_PI / 6 * (0.5 + k)), glob.coords[glob.selectedTile][1] + glob.sideLength * cos(2 * M_PI / 6 * (0.5 + k)));
		}
	}
	for (int i = 0; i < glob.count; i++)	// Numbers
	{
		string result, result2;
		stringstream convert, convert2;
		convert << i;
		result = convert.str();
		const char *c = result.c_str();

		convert2 << glob.state[i];
		result2 = convert2.str();
		const char *c2 = result2.c_str();

		cairo_set_source_rgb(cr, 1, 1, 1);
		cairo_set_font_size(cr, glob.sideLength / 2.0);
		if (i < 10)
		{
			cairo_move_to(cr, glob.coords[i][0] - glob.sideLength / 2.0 / 3.0, glob.coords[i][1] + glob.sideLength / 2.0 / 3.0);
		}
		else if (i < 100)
		{
			cairo_move_to(cr, glob.coords[i][0] - glob.sideLength / 2.0 / 3.0 * 2.0, glob.coords[i][1] + glob.sideLength / 2.0 / 3.0);
		}
		cairo_show_text(cr, c);
		
		cairo_set_source_rgb(cr, 0, 0, 1);
		cairo_move_to(cr, glob.coords[i][0] - glob.sideLength / 2.0 / 3.0, glob.coords[i][1] + glob.sideLength / 2.0 / 3.0 + glob.sideLength / 2.0);
		cairo_show_text(cr, c2);
	}
}
static void button_clicked(GtkWidget* widget, gpointer data)
{
	system("reset");
	for (int i = 0; i < glob.count; i++)
	{
		printf("%i: (%f, %f, %i)\n", i, glob.coords[i][0], glob.coords[i][1], glob.path[i], glob.state[i]);
	}
	getNeighbors();
	for (int n = 0; n < glob.count; n++)
	{
		printf("Base Station: %i\n\tCan be deleted: %s\n\tNeighbors: ", n, (deletionValid(n) ? "true" : "false"));
		for (int i = 0; i < glob.neighbors[n].size(); i++)
		{
			printf("(%i,%i)", glob.neighbors[n][i].first, glob.neighbors[n][i].second);
			if(i < glob.neighbors[n].size() - 1)
			{
				printf(", ");
			}
		}	
		printf("\n");
	}
	printf("\n");
	goToSimParams();
}
static gboolean mouse_moved(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	if (event -> type == GDK_MOTION_NOTIFY)
 	{
  		GdkEventMotion* e = (GdkEventMotion*)event;
		glob.mouseX = (guint)e -> x;
		glob.mouseY = (guint)e -> y;
		gtk_widget_queue_draw(widget);
  	}
}
static gboolean mouse_clicked(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	bool changeScale = false;
	if (event->button == 1) //Left Mouse Click
	{	
		double dY = (glob.coords[glob.selectedTile][1] - event -> y);
		double dX = (event -> x - glob.coords[glob.selectedTile][0]);
		
		double param = dY / dX;
		double slope = atan(param) * 180.0 / M_PI;
		double setX, setY;
		int setPath;
		double dist = sqrt(dY * dY + dX * dX);

		bool changedTile = false;
		double distance = sqrt(dY * dY + dX * dX);
		double newdY, newdX, newDistance;
		for(int i = 0; i < glob.count; i++)
		{
			newdY = (glob.coords[i][1] - event -> y);
			newdX = (event -> x - glob.coords[i][0]);
			
			newDistance = sqrt(newdY * newdY + newdX * newdX);
			if (distance > newDistance)
			{
				distance = newDistance;
				if (distance < glob.sideLength * sqrt(3) / 2)	// If click is inside hex
				{
					glob.selectedTile = i;
					changedTile = true;
				}
			}
		}
		if(!changedTile)
		{
			if (dist > glob.sideLength * sqrt(3) / 2)
			{
				if (abs(slope) >= 60)
				{
				 	if (dY < 0)	// Bottom
				  	{
					 	setX = glob.coords[glob.selectedTile][0];
					 	setY = glob.coords[glob.selectedTile][1] + glob.sideLength * sqrt(3);
					 	setPath = 0;
					}
					else  // Top
					{
					 	setX = glob.coords[glob.selectedTile][0];
					 	setY = glob.coords[glob.selectedTile][1] - glob.sideLength * sqrt(3);
					 	setPath = 3;
					}
				}
				else
				{
				  	if (dY < 0)
				  	{
				    	if (dX > 0)	// Bottom Right
				    	{
					    	setX = glob.coords[glob.selectedTile][0] + glob.sideLength * 1.5;
					    	setY = glob.coords[glob.selectedTile][1] + glob.sideLength * sqrt(3) / 2;
					    	setPath = 5;
					  	}
					  	else	// Bottom Left
					  	{
					    	setX = glob.coords[glob.selectedTile][0] - glob.sideLength * 1.5;
					    	setY = glob.coords[glob.selectedTile][1] + glob.sideLength * sqrt(3) / 2;
					    	setPath = 1;
					  	}
				  	}
				  	else
				  	{
				    	if (dX > 0)	// Top Right
				    	{
					    	setX = glob.coords[glob.selectedTile][0] + glob.sideLength * 1.5;
					    	setY = glob.coords[glob.selectedTile][1] - glob.sideLength * sqrt(3) / 2;
					    	setPath = 4;
					  	}
					  	else	// Top Left
					  	{
					    	setX = glob.coords[glob.selectedTile][0] - glob.sideLength * 1.5;
					    	setY = glob.coords[glob.selectedTile][1] - glob.sideLength * sqrt(3) / 2;
					    	setPath = 2;
					  	}
				  	}
				}
				bool pointExists = false;
				for (int i = 0; i < glob.count; i++)
				{
					if (round(glob.coords[i][0]) == round(setX) && round(glob.coords[i][1]) == round(setY))
					{
						pointExists = true;
						break;
					}
				}
				if (!pointExists)
				{			
					vector<double> test;
					test.push_back(setX);
					test.push_back(setY);
					glob.coords.push_back(test);

					glob.state.push_back((int)0);

					glob.path[glob.count - 1] = setPath;

					glob.selectedTile = glob.count;
					glob.count += 1;
					changeScale = true;
				}
			}
			else	// If inside the hexagon, cycle states
			{
				if(glob.state[glob.selectedTile] >= 3)
				{
					glob.state[glob.selectedTile] = 0;	
				}
				else
				{
					glob.state[glob.selectedTile] += 1;
				}
			}
		}
  	}
	if (event->button == 3)	// Right Mouse Click
	{
		double dY = (glob.coords[glob.selectedTile][1] - event -> y);
		double dX = (event -> x - glob.coords[glob.selectedTile][0]);
		
		double param = dY / dX;
		double slope = atan(param) * 180.0 / M_PI;
		double distance = sqrt(dY * dY + dX * dX);
		double newdY, newdX, newDistance;
		bool foundClick = false;
		for(int i = 0; i < glob.count; i++)
		{
			if(!foundClick)
			{
				newdY = (glob.coords[i][1] - event -> y);
				newdX = (event -> x - glob.coords[i][0]);
				
				newDistance = sqrt(newdY * newdY + newdX * newdX);
				printf("Pass: %i\n", i);
				if (distance > newDistance)
				{
					distance = newDistance;
					if (round(distance) < round(glob.sideLength * sqrt(3) / 2))	// If click is inside hex
					{
						foundClick = true;
						if(glob.count > 1)
						{
							getNeighbors();
							if(deletionValid(i))
							{
								printf("Deleting: %i\n", i);
								glob.coords.erase(glob.coords.begin() + i);
								glob.state.erase(glob.state.begin() + i);
								glob.count -= 1;
								glob.selectedTile = 0;
								changeScale = true;
							}
							else
							{
								printf("Deletion invalid\n");
							}
						}
						else
						{
							printf("Last tile cannot be deleted\n");
						}
					}
				}
				else if (round(distance) < round(glob.sideLength * sqrt(3) / 2))	// If click is inside hex
				{
					foundClick = true;	
					if(glob.count > 1)
					{
						getNeighbors();		
						if(deletionValid(i))
						{
							printf("Deleting: %i\n", glob.selectedTile);
							glob.coords.erase(glob.coords.begin() + glob.selectedTile);
							glob.state.erase(glob.state.begin() + i);	
							glob.count -= 1;
							glob.selectedTile = 0;
							changeScale = true;
						}
						else
						{
							printf("Deletion invalid\n");
						}
					}
					else
					{
						printf("Last tile cannot be deleted\n");
					}
				}
			}
			else
			{
				break;
			}
		}
	}
	if (changeScale)	// Work on this part, needs to scale larger if tiles are deleted
	{		
		float ratio = 1.0;
		float minX, maxX, minY, maxY, difX, difY, numX, numY;
		minX = glob.coords[0][0];
		maxX = glob.coords[0][0];
		minY = glob.coords[0][1];
		maxY = glob.coords[0][1];
		for(int i = 1; i < glob.count; i++)
		{
			if (minX > glob.coords[i][0])
			{
				minX = glob.coords[i][0];
			}
			if (maxX < glob.coords[i][0])
			{
				maxX = glob.coords[i][0];
			}
			if (minY > glob.coords[i][1])
			{
				minY = glob.coords[i][1];
			}
			if (maxY < glob.coords[i][1])
			{
				maxY = glob.coords[i][1];
			}
		}
		difX = abs(maxX - minX);
		difY = abs(maxY - minY);
		numX = ceil(difX / (glob.sideLength*sqrt(3) / 2)) / 2 + 1;
		numY = ceil(difY / (glob.sideLength*sqrt(3) / 2)) / 2 + 1;
		printf("NumX: %f\nNumY: %f\n", numX, numY);

		double prevSideLength = glob.sideLength;

		if(glob.count == 1)	// Good
		{
			ratio = (glob.screenHeight * 0.25) / glob.sideLength;
			minX = glob.screenWidth * 0.95 / 2.0;
			maxX = glob.screenWidth * 0.95 / 2.0;
			minY = glob.screenHeight * 0.95 / 2.0;
			maxY = glob.screenHeight * 0.95 / 2.0;
			difX = 0;
			difY = 0;
			glob.coords[0][0] = glob.screenWidth * 0.95 / 2.0;
			glob.coords[0][1] = glob.screenHeight * 0.95 / 2.0;
		}
		// If shrinking
		else if ((glob.sideLength*sqrt(3) * numX) > glob.screenWidth * 0.95 || (glob.sideLength*sqrt(3) * numY) > glob.screenHeight * 0.95)
		{
			printf("Shrinking\n");
			bool keepShrinking = true;
			while(keepShrinking)
			{
				if ((glob.sideLength*sqrt(3) * numX) > glob.screenWidth * 0.95 || (glob.sideLength*sqrt(3) * numY) > glob.screenHeight * 0.95)
				{
					glob.sideLength *= 0.999;	
				}
				else
				{
					keepShrinking = false;
				}
			}
		}
		else
		{
			printf("Growing\n");
			bool keepGrowing = true;
			while(keepGrowing)
			{
				if (!((glob.sideLength*sqrt(3) * numX) > glob.screenWidth * 0.95 || (glob.sideLength*sqrt(3) * numY) > glob.screenHeight * 0.95))
				{
					glob.sideLength *= 1.001;	
				}
				else
				{
					keepGrowing = false;
				}
			}
		}
		ratio = glob.sideLength / prevSideLength;
		for (int i = 0; i < glob.count; i++)
		{
			glob.coords[i][0] = (glob.coords[i][0] - glob.screenWidth * 0.95 / 2.0) * ratio + glob.screenWidth * 0.95 / 2.0 - (minX + difX / 2.0 - glob.screenWidth * 0.95 / 2.0);
			glob.coords[i][1] = (glob.coords[i][1] - glob.screenHeight * 0.95 / 2.0) * ratio + glob.screenHeight * 0.95 / 2.0 + (glob.screenHeight * 0.95 / 2.0 - (maxY - difY / 2.0));
		}
	}
	gtk_widget_queue_draw(widget);
  	return TRUE;
}
static gboolean on_draw_event(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
 	drawHex(cr);
 	return FALSE;
}
static void getNeighbors()
{
	for (int k = 0; k < glob.count; k++)	// Wipes the neighbors so it doesn't retain old neighbors
	{
		glob.neighbors[k].erase(glob.neighbors[k].begin(), glob.neighbors[k].end());
	}
	for (int n = 0; n < glob.count; n++)
	{
		for (int i = 0; i < glob.count; i++)
		{
			if(i != n && round(distance(glob.coords[i][0], glob.coords[i][1], glob.coords[n][0], glob.coords[n][1])) == round(glob.sideLength*sqrt(3)))
			{
				glob.neighbors[n].push_back(make_pair(i,side(glob.coords[i][0], glob.coords[i][1], glob.coords[n][0], glob.coords[n][1])));
			}
		}
	}
}
static bool deletionValid(int tile)
{
	bool* nodeExplored = new bool[glob.count];
	for (int n = 0; n < glob.count; n++)
		nodeExplored[n] = false;

	std::vector<int> nodesToExplore = {(tile + 1) % glob.count};	//element, guaranteed to exist.

	int N;
	while (!nodesToExplore.empty())
	{
		N = nodesToExplore.back();
		nodesToExplore.pop_back();

		if(N == tile)
			continue;

		nodeExplored[N] = true;

		for (int c = 0; c < glob.neighbors[N].size(); c++)
		{
			if(!nodeExplored[glob.neighbors[N][c].first])
				nodesToExplore.push_back(glob.neighbors[N][c].first);
		}
	}
	for (int n = 0; n < glob.count; n++)
		if(nodeExplored[n] == false && n != tile)
			return false;

	return true;
}
static float distance(double x1, double y1, double x2, double y2)
{
	double dY = y2 - y1;
	double dX = x2 - x1;
	double distance = sqrt(dY * dY + dX * dX);
	return distance;
}
static int side(double x1, double y1, double x2, double y2)
{
	double dY = y1 - y2;
	double dX = x1 - x2;
	double angle = atan(dY/dX);
	if(dX > 0)
		angle = (angle > 0 ? angle : angle + 2 * M_PI);
	else
		angle = M_PI - angle;
	int side = ((int)(angle/(M_PI/3)));
	return side;
}
void addParams()
{
	// get the text from each entry box and add the text to the glob structure
	try
	{
		glob.bsLen = stoi(gtk_entry_get_text(GTK_ENTRY(entryBoxes.baseStationSide)));
		glob.antNum = stoi(gtk_entry_get_text(GTK_ENTRY(entryBoxes.antennaNumber)));
		glob.transNum = stoi(gtk_entry_get_text(GTK_ENTRY(entryBoxes.transceiverNum)));
		glob.dRateMax = stoi(gtk_entry_get_text(GTK_ENTRY(entryBoxes.maxDataRate)));
		glob.uePerAnt = stoi(gtk_entry_get_text(GTK_ENTRY(entryBoxes.userEquipPerAntenna)));
		glob.simLen = stoi(gtk_entry_get_text(GTK_ENTRY(entryBoxes.simulationLength)));
		glob.simNum = stoi(gtk_entry_get_text(GTK_ENTRY(entryBoxes.simulationNumber)));
		glob.simStartNum = stoi(gtk_entry_get_text(GTK_ENTRY(entryBoxes.simulationStart)));
		glob.bufSize = stoi(gtk_entry_get_text(GTK_ENTRY(entryBoxes.bufferSize)));
		
		// double
		glob.transDist = stod(gtk_entry_get_text(GTK_ENTRY(entryBoxes.transceiverDist)));
		
		// strings
		glob.simName = gtk_entry_get_text(GTK_ENTRY(entryBoxes.simulationSaveName));
	}
	catch(const exception& ex)
	{
		cout << "Some values entered may not be valid; default parameters are substituted for these values" << endl;
	}
	
}
void getDimensions()
{
	// create screenGeo object that contains window length and width
	GdkRectangle screenGeo;
	GdkDisplay* gdkDisplay = gdk_display_get_default();
	GdkMonitor* monitor = gdk_display_get_primary_monitor(gdkDisplay);
	gdk_monitor_get_geometry(monitor, &screenGeo);
	
	// Add screen dimensions to screen struct object to be used across the project
	SCREEN.WIDTH = screenGeo.width;
	SCREEN.HEIGHT = screenGeo.height;
}

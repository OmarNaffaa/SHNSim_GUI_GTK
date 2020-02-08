#include <iostream>
#include <gtk/gtk.h>

using namespace std;

// define structure to hold windows used for project
typedef struct
{
	GtkWidget* DrawingWindow;
	GtkWidget* SimParamWindow;
	GtkWidget* DiagnosticsWindow;
	
} Windows;

// define structure to hold window dimensions
typedef struct
{
	int width;
	int height;
	
} Screen;

// window setup function prototypes
void setUpDrawingWindow();
void setUpSimParamWindow();
void setUpDiagnosticsWindow();
// navigation function prototypes - used to change windows
void goToSimParams();
void goToDiagnostics();
void backToDrawingStage();
// Utility functions
void getDimensions();

Windows windowList;
Screen screen;

int main(int argc, char** argv)
{
	// initialize gtk	
	gtk_init(&argc, &argv);
	
	setUpSimParamWindow();
	gtk_widget_show_all(windowList.SimParamWindow); // change to stage1 window
	
	gtk_main();
	return 0;
}

void setUpDrawingWindow()
{
	// add Cameron's code here
}

void setUpSimParamWindow()
{
	// create stage 2 window and configure window properties
	GtkWidget* s2Window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(s2Window), "Self-Healing Simulator");
	gtk_window_set_default_size(GTK_WINDOW(s2Window), screen.width, screen.height);
	g_signal_connect(s2Window, "delete-event", G_CALLBACK(gtk_main_quit), NULL);
	
	// copy window into struct objcet for global use across functions
	windowList.SimParamWindow = s2Window;
	
	// create input labels and text boxes from stage 2 of C# code
	GtkWidget *bsSide, *numAntenna, *numTransceivers, *distTransceivers, *maxDR, *uePerAntenna; // labels
	GtkWidget *bsSideTxt, *numAntennaTxt, *numTransceiversTxt, *distTransceiversTxt, *maxDRTxt, *uePerAntennaTxt; // textbox
	
	// create input labels and text boxes from stage 3 of C# code
	GtkWidget *simLength, *simNum, *simStart, *simSaveName; // labels
	GtkWidget *simLengthTxt, *simNumTxt, *simStartTxt, *simSaveNameTxt; // textboxes
	
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
	
	backToS1Btn = gtk_button_new_with_label("Back");
	runSimBtn = gtk_button_new_with_label("Run Simulation");
	
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
		
		// title
		gtk_style_context_add_provider(gtk_widget_get_style_context(title), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
	}
}

void setUpDiagnosticsWindow()
{
	// TODO
}

void goToSimParams()
{
	gtk_widget_show_all(windowList.SimParamWindow);
	gtk_widget_hide_on_delete(windowList.DrawingWindow);
}

void goToDiagnostics()
{
	gtk_widget_show_all(windowList.DiagnosticsWindow);
	gtk_widget_hide_on_delete(windowList.SimParamWindow);
}

void backToDrawingStage()
{
	gtk_widget_show_all(windowList.DrawingWindow);
	gtk_widget_hide_on_delete(windowList.SimParamWindow);
}

void getDimensions()
{
	// create screenGeo object that contains window length and width
	GdkRectangle screenGeo;
	GdkDisplay* gdkDisplay = gdk_display_get_default();
	GdkMonitor* monitor = gdk_display_get_primary_monitor(gdkDisplay);
	gdk_monitor_get_geometry(monitor, &screenGeo);
	
	// Add screen dimensions to screen struct object to be used across the project
	screen.width = screenGeo.width;
	screen.height = screenGeo.height;
}

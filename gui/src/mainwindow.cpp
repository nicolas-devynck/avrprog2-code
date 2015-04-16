#include <QDebug>
#include <QFileDialog>
#include <QDir>
#include <QProcess>
#include <QVariant>
#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // create the menubar
    createActions();    
    createMenus();

    setWindowTitle(tr("AVR LinuxFlash [V0.1]"));
    setCentralWidget(ui->frame);


    // connect the buttons to their appropriate slots
    connect( ui->selectFile, SIGNAL( clicked() ), this, SLOT( loadhexFile()    ) );
    connect( ui->btn_write,  SIGNAL( clicked() ), this, SLOT( write()          ) );

    // connect the combo box slots
    connect( ui->chooseDevice,    SIGNAL( currentTextChanged( QString )  ), this, SLOT( deviceChosen())    );
    connect( ui->chooseFrequency, SIGNAL( currentIndexChanged( QString ) ), this, SLOT( frequencyChosen()) );

    // connect the text edit box
    connect( ui->enterFuseBits, SIGNAL( editingFinished() ), this, SLOT( fuseBitsChanged() ) );

    // populate the chooseDevice combo box
    createDeviceList();

}

MainWindow::~MainWindow()
{
    delete ui;
}

// ****************************************************************************
void MainWindow::createDeviceList()
{
    /* ** populate chooseDevice combo box with mcu choices from /usr/local/share/avrprog2 ** */

        QDir mcuList( "/usr/local/share/avrprog2" );  // set the directory to use
        QStringList filters;                          // variable to hold our filter setting
        filters << "*.xml";                           // load our setting onto our variable
        mcuList.setNameFilters( filters );            // put our setting into our directory
        mcuList.setFilter( QDir::Files );             // set the directory to files only

        QStringList mcuFiles = mcuList.entryList();   // get the list of files with .xml on the end
        QString fileName;                             // temporary container for manipulating through our array
        QStringList mcuNames;                         // list to hold the modified file names
        foreach ( fileName , mcuFiles )               // use fileName to go through each entry in mcuFiles
        {
            // take the .xml off the end of the fileName and add it to the mcuNames array
            mcuNames << fileName.remove( QRegExp("(.xml)$") );
        }

        ui->chooseDevice->addItems( mcuNames );       // assign the list of modified file names to the combo box

        if ( ui->chooseDevice->currentText() == "")
        {
        ui->commandText->setText("No Devices Found");
        ui->commandText->setVisible( true );
        }
        else
        {
            ui->display_mcuName->setText("");
        }

        populateCommand();
}

// ****************************************************************************
void MainWindow::createActions()
{

    // fileMenu actions ------------------------
    loadhexAct = new QAction(tr("Select File"), this);
    loadhexAct->setStatusTip(tr("Select a Hex file"));
    connect(loadhexAct, SIGNAL(triggered()), this, SLOT(loadhexFile()));

    exitAct = new QAction(tr("Exit"), this);
    exitAct->setStatusTip(tr("Exit Program"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(exitFile()));


    // aboutMenu actions -----------------------
    aboutAct = new QAction(tr("About"), this);
    aboutAct->setStatusTip(tr("About this project"));
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

}

// ****************************************************************************
void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
        fileMenu->addAction( loadhexAct );
        fileMenu->addSeparator();
        fileMenu->addAction( exitAct );

    aboutMenu = menuBar()->addMenu(tr("&About"));
        aboutMenu->addAction( aboutAct );

}


// ****************************************************************************
void MainWindow::write()
{
    if ( !(ui->enterFuseBits->text() == "c0,d9") )
    {
        switch( QMessageBox::warning(
                    this,
                    tr("Potential Danger!!"),
                    tr("Fuse and/or Lock bits changed! \n   * Continue write operation? *"),

                       QMessageBox::Yes |
                       QMessageBox::No )
               )
            {
                case QMessageBox::No:
                ui->enterFuseBits->setText( "c0,d9");
                ui->display_fuseBits->setText(" --fuses w:c0,d9");
                populateCommand();
                return;
                break;
                case QMessageBox::Yes:
                break;
                default:
                break;
            }
    }

    // set up the QString for the QProcess
    QString command = ui->commandText->text();

    QMessageBox test;
    test.setWindowTitle("command text test");
    test.setText(command);
    test.exec();

    //QProcess *writeToMcu;
    //writeToMcu->execute(command);
}

// ****************************************************************************
void MainWindow::loadhexFile()
{    

    QStringList filters;                              // variable to hold our filter setting
    filters << "*.hex" << "*.ihex" << "*.elf";        // set the filter to our specs

    QFileDialog loadFile( this );                     // create the dialog box
    loadFile.setNameFilters( filters );               // set the filters for the dialog box
    loadFile.setFileMode( QFileDialog::ExistingFile );// set the mode of the QFileDialog

        if ( loadFile.exec() ) // if the dialog box exists
        {
            QStringList fileNames = loadFile.selectedFiles();// get the selected file
            ui->display_fileName->setText( " --flash w:" + fileNames[0] );       // display the path and file
            populateCommand();
        }

}

// ****************************************************************************
void MainWindow::exitFile()
{
   close();
}

// ****************************************************************************
void MainWindow::about()
{
    QMessageBox abouttext;
    abouttext.setWindowTitle("About LinuxFlash");
    abouttext.setText("AVR LinuxFlash accesses AVRProg2 with the command line. \
 It generates the command based on the GUI selections and sends it to the board for you.");
    abouttext.exec();

}

// ****************************************************************************
void MainWindow::deviceChosen()
{
    // set text of device indicator label
    ui->display_mcuName->setText( "-m " + ui->chooseDevice->currentText() );
    populateCommand();
}

// ****************************************************************************
void MainWindow::frequencyChosen()
{
    QString freqTxt = ui->chooseFrequency->currentText();  // get the selected frequency
    double temp = freqTxt.toDouble();                      // turn the text into a double
    long freq = ( long )( temp*1000000 );                  // multiply the double and cast it to a long
    freqTxt = freqTxt.number( freq,10 );                   // turn the long into a string

    // set text of device indicator label
    ui->display_frequency->setText( " -f " + freqTxt );
    populateCommand();
}

void MainWindow::populateCommand()
{
    if ( ui->commandText->text() == "No Devices Found" )
    {
        return;
    }

    ui->commandText->setText( "avrprog2 " +
                               ui->display_mcuName->text() +
                               ui->display_frequency->text() +
                               ui->display_fileName->text() +
                               ui->display_fuseBits->text() );


    if ( ui->display_mcuName->text() !=  "" &&
         ui->display_frequency->text() != "" &&
         ui->display_fileName->text() != "" &&
         ui->display_fuseBits->text() != "" )
    {
        QString write = "Write to MCU";
        ui->commandText->setVisible( true );
        ui->btn_write->setText( write );
        ui->btn_write->setEnabled(true);
    }
    else
    {
        QString choose = "Enter Choices";
        ui->commandText->setVisible( false );
        ui->btn_write->setEnabled(false);
        ui->btn_write->setText( choose );
    }

}

// ****************************************************************************
void MainWindow::fuseBitsChanged()
{

    QString checkThis = ui->enterFuseBits->text();        // get the user input
    QRegExp withThis( "^[0-9a-fA-F]{2},[0-9a-fA-F]{2}" ); // set the filter
    if ( !(withThis.exactMatch( checkThis )) )              // check if the input is valid
    {
       ui->display_fuseBits->setText( " --fuses w:c0,d9" );
       ui->enterFuseBits->setText( "c0,d9" );
        return;
    }


    ui->display_fuseBits->setText( " --fuses w:" + ui->enterFuseBits->text() );

}

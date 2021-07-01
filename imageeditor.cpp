#include "imageeditor.h"
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QColorSpace>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSpinBox>
#include <QComboBox>
#include <QColorDialog>
#include <QRubberBand>
#include <QSizeGrip>

imageEditor::imageEditor(QWidget *parent)
    : QMainWindow(parent)
{
    wgt=new QWidget;
    imageLabel=new QLabel();

    //Определенние надписей
    previewLabel = new QLabel("Предпросмотр:"); //надпись для предпросмотра изображения
    autoLabel = new QLabel("Преобразовать в монохромное изображение:"); //надпись автоматического преобразования
    binarOptionsLabel = new QLabel("Выбрать опции бинаризации:");//надпись кастомной бинаризации
    changePaletteLabel = new QLabel("Изменить палитру:");//надпись изменения палитры
    autoButton = new QPushButton("Автоматическая бинаризация");

    // Группировка виджетов бинаризации
    contrastVal = new QSpinBox;
    contrastVal->setRange(-63,63);//минимум и максимум контраста
    //QPushButton* binarColourButton = new QPushButton("Цвет");
    binarColourButton=new QComboBox;
    binarizeButton = new QPushButton("Бинаризировать");
    binar=new QHBoxLayout;
    binar->addWidget(contrastVal);
    binar->addWidget(binarColourButton);
    binar->addWidget(binarizeButton);

    //Определение списка цветов для бинаризации
    binarColourList<<"Чёрный"<<"Красный"<<"Жёлтый"<<"Зелёный"<<"Голубой"<<"Фиолетовый";
    binarColourButton->addItems(binarColourList);
    binarColourButton->setEditable(false);

    //Группировка виджетов для изменения палитры
    fColourLabel = new QLabel("Исходный цвет:");
    sColourLabel = new QLabel("Желаемый цвет:");
    fColourChoose=new QComboBox;
    sColourChoose = new QPushButton("Цвет");
    changeColourButton = new QPushButton("Заменить");
    previewButton = new QPushButton("Предпросмотр");
    paletteGrid = new QGridLayout;
    paletteGrid->addWidget(fColourLabel, 0, 0);
    paletteGrid->addWidget(sColourLabel, 0, 1);
    paletteGrid->addWidget(fColourChoose, 1, 0);
    paletteGrid->addWidget(sColourChoose, 1, 1);
    paletteGrid->addWidget(previewButton,2,0);
    paletteGrid->addWidget(changeColourButton,2,1);

    //Левая часть приложения
    leftLayout = new QVBoxLayout;
    leftLayout->addWidget(previewLabel);//добавляем надпись предпросмотра в левое расположение
    leftLayout->addWidget(imageLabel);//добавляем изображение в левое расположение
    leftLayout->addStretch(1);

    //Правая часть приложения
    rightLayout = new QVBoxLayout;
    rightLayout->setMargin(20);
    rightLayout->setSpacing(10);
    rightLayout->addWidget(autoLabel);
    rightLayout->addWidget(autoButton);
    rightLayout->addWidget(binarOptionsLabel);
    rightLayout->addLayout(binar);
    rightLayout->addWidget(changePaletteLabel);
    rightLayout->addLayout(paletteGrid);
    rightLayout->addWidget(changeColourButton);
    rightLayout->addStretch(1);

    //main part widget
    mainLayout = new QHBoxLayout;
    mainLayout->setMargin(20);
    mainLayout->setSpacing(10);
    mainLayout->addLayout(leftLayout);
    mainLayout->addStretch(1);
    mainLayout->addLayout(rightLayout);

    wgt->setLayout(mainLayout);
    setCentralWidget(wgt);

    //Подключение кнопок к слотам
    connect(autoButton, SIGNAL (released()), this, SLOT (autoBinarButton()));
    connect(binarizeButton, SIGNAL (released()), this, SLOT (binarButton()));
    connect(sColourChoose, SIGNAL (released()), this, SLOT (wishColour()));
    connect(changeColourButton, SIGNAL (released()), this, SLOT (change()));
    connect(previewButton,SIGNAL (released()), this, SLOT (preview()));

    //Деактивация кнопок, до открытия изображения
    autoButton->setEnabled(false);
    binarizeButton->setEnabled(false);
    changeColourButton->setEnabled(false);
    previewButton->setEnabled(false);
    sColourChoose->setEnabled(false);

    band = new Resizable_rubber_band(imageLabel);
    band->setVisible(false);

    createActions();
    createMenus();
    setWindowTitle(tr("Image Editor"));
    resize(500, 400);
}

void imageEditor::open()
{
    QString str = QFileDialog::getOpenFileName(this, tr("Open Dialog"), "", "*.png *.jpg *.jpeg");
    img.load(str);
    if(!img.isNull())
    {
        saveAct->setEnabled(true);
        autoButton->setEnabled(true);
        binarizeButton->setEnabled(true);
        selectAreaAct->setEnabled(true);
        sColourChoose->setEnabled(true);
    }
    else
    {
        saveAct->setEnabled(false);
        autoButton->setEnabled(false);
        binarizeButton->setEnabled(false);
        selectAreaAct->setEnabled(false);
        previewButton->setEnabled(false);
        changeColourButton->setEnabled(false);
        sColourChoose->setEnabled(false);
    }
    imageLabel->setPixmap(QPixmap::fromImage(img));
    updateList();
}

void imageEditor::save()
{

    QString strFilter;
    QString str = QFileDialog::getSaveFileName(this,tr("Save new image"),"New image","*.png ;; *.jpg ;; *.jpeg",&strFilter);
    if (!str.isEmpty()) {
        if (strFilter.contains("jpg")) {
            img.save(str, "JPG");
        }
        else if (strFilter.contains("jpeg")) {
            img.save(str, "JPEG");
        }
        else {
            img.save(str, "PNG");
        }
    }

}

void imageEditor::cut()
{
    QRect rect(band->pos(),band->size());
    QImage cropped = img.copy(rect);
    img=cropped.copy();
    imageLabel->setPixmap(QPixmap::fromImage(img));
    updateList();
    bandUpdate();
    selectAreaAct->setChecked(false);
    selectArea();
}

void imageEditor::selectArea()
{
    bool selectAreaBool = selectAreaAct->isChecked();
    if(selectAreaBool)
    {
        bandUpdate();
        band->setVisible(true);
        cutAct->setEnabled(true);
    }
    else
    {
        band->setVisible(false);
        cutAct->setEnabled(false);
    }
}

void imageEditor::autoBinarButton()
{
    QSize sizeImage = img.size();
    int width = sizeImage.width(), height = sizeImage.height();

    QRgb color;
    for (int f1=0; f1<width; f1++) {
        for (int f2=0; f2<height; f2++) {
            color = img.pixel(f1, f2);
            int gray = qGray(color);
            img.setPixel(f1, f2, qRgb(gray, gray, gray));
        }
    }
    imageLabel->setPixmap(QPixmap::fromImage(img));
    updateList();
}

void imageEditor::binarButton()
{
    int contrast=contrastVal->value();
    int color=binarColourButton->currentIndex();
    for (int w = 0; w < img.rect().right(); w++)
    {
        for (int h = 0; h < img.rect().bottom(); h++)
        {
            QColor col(img.pixel(w, h));
            int r, g, b;
            col.getRgb(&r, &g, &b);
            int val = (r+g+b)/3;
            val = val*(256 - 63*2 - contrast*2)/256;
            val = val*256/(256 - 63*2 - contrast*2);
            if (color == 0) { //Black
                col.setRgb(val, val, val);
                img.setPixel(w, h, col.rgb());
            } else if (color == 1) { //Red
                col.setRgb(255, val, val);
                img.setPixel(w, h, col.rgb());
            } else if (color == 2) { //Yellow
                col.setRgb(255, 255, val);
                img.setPixel(w, h, col.rgb());
            } else if (color == 3) { //Green
                col.setRgb(val, 255, val);
                img.setPixel(w, h, col.rgb());
            } else if (color == 4) { //Blue
                col.setRgb(val, val, 255);
                img.setPixel(w, h, col.rgb());
            } else if (color == 5) { //Violet
                col.setRgb(255, val, 255);
                img.setPixel(w, h, col.rgb());
            }
        }
    }
    imageLabel->setPixmap(QPixmap::fromImage(img));
    updateList();
}

void imageEditor::wishColour()
{
    secColor=QColorDialog::getColor();
    if(secColor.isValid()){
        previewButton->setEnabled(true);
        changeColourButton->setEnabled(true);
    }
    QPalette pal = sColourChoose->palette();
    pal.setColor(QPalette::Button, secColor);
    sColourChoose->setAutoFillBackground(true);
    sColourChoose->setPalette(pal);
    sColourChoose->update();
}

void imageEditor::change()
{
    QSize sizeImage = img.size();
    int width = sizeImage.width(), height = sizeImage.height();
    QColor color;
    for (int f1=0; f1<width; f1++) {
        for (int f2=0; f2<height; f2++) {
            color = img.pixel(f1, f2);
            if(color==originColours[fColourChoose->currentIndex()]){
                img.setPixelColor(f1, f2, secColor);
            }
        }
    }
    imageLabel->setPixmap(QPixmap::fromImage(img));
    updateList();
}

void imageEditor::preview()
{
    QLabel* previewLabel = new QLabel();
    QImage previewImage = img.copy();
    QSize sizeImage = previewImage.size();
    int width = sizeImage.width(), height = sizeImage.height();
    QColor color;
    for (int f1=0; f1<width; f1++) {
        for (int f2=0; f2<height; f2++) {
            color = previewImage.pixel(f1, f2);
            if(color==originColours[fColourChoose->currentIndex()]){
                previewImage.setPixelColor(f1, f2, secColor);
            }
        }
    }
    previewLabel->setPixmap(QPixmap::fromImage(previewImage));
    previewLabel->show();
}

void imageEditor::createActions()//создание действий
{
    openAct = new QAction(tr("&Открыть..."), this);
    openAct->setShortcut(tr("Ctrl+O"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));
    saveAct=new QAction(tr("&Сохранить как..."),this);
    saveAct->setShortcut(tr("Ctrl+S"));
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));
    saveAct->setEnabled(false);
    exitAct = new QAction(tr("&Выход"), this);
    exitAct->setShortcut(tr("Ctrl+Q"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));
    cutAct = new QAction(tr("&Обрезать"),this);
    cutAct->setEnabled(false);
    connect(cutAct, SIGNAL(triggered()), this, SLOT(cut()));
    selectAreaAct = new QAction(tr("&Выделить область"),this);
    selectAreaAct->setEnabled(false);
    selectAreaAct->setCheckable(true);
    connect(selectAreaAct, SIGNAL(triggered()), this, SLOT(selectArea()));
}
void imageEditor::createMenus()//добавляем меню
{
    fileMenu = new QMenu(tr("&Файл"), this);
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);
    fileMenu->addAction(exitAct);
    menuBar()->addMenu(fileMenu);

    cutMenu = new QMenu(tr("&Обработка"),this);
    cutMenu->addAction(selectAreaAct);
    cutMenu->addAction(cutAct);
    menuBar()->addMenu(cutMenu);
}
void imageEditor::updateList()
{
    //Определение списка содержащихся цветов (цветовой палитры)
    originColours.clear();
    fColourChoose->clear();
    QSize sizeImage = img.size();
    int width = sizeImage.width(), height = sizeImage.height();
    QColor color;
    for (int f1=0; f1<width; f1++) {
        for (int f2=0; f2<height; f2++) {
            color = img.pixel(f1, f2);
            if(originColours.contains(color.name())==false){
                originColours.push_back(color.name());
            }
        }
    }
    fColourChoose->addItems(originColours);
}

void imageEditor::bandUpdate()
{
    QSize sizeImage = img.size();
    QSize sizeBand= band->size();
    int width = sizeImage.width(), height = sizeImage.height();
    band->resize(width/4, height/4);
    band->move((width-sizeBand.width())/2, (height-sizeBand.height())/2);
    band->setMinimumSize(100, 100);
}

//Выделить необходимую часть
Resizable_rubber_band::Resizable_rubber_band(QWidget *parent) : QWidget(parent) {
    setWindowFlags(Qt::SubWindow);
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    QSizeGrip* grip1 = new QSizeGrip(this);
    QSizeGrip* grip2 = new QSizeGrip(this);
    layout->addWidget(grip1, 0, Qt::AlignLeft | Qt::AlignTop);
    layout->addWidget(grip2, 0, Qt::AlignRight | Qt::AlignBottom);
    rubberband = new QRubberBand(QRubberBand::Rectangle, this);
    rubberband->move(0, 0);
    rubberband->show();
}

void Resizable_rubber_band::resizeEvent(QResizeEvent *) {
    rubberband->resize(size());
}

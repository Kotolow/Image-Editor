#ifndef IMAGEEDITOR_H
#define IMAGEEDITOR_H

#include <QMainWindow>

class QAction;
class QLabel;
class QPushButton;
class QSpinBox;
class QComboBox;
class QHBoxLayout;
class QVBoxLayout;
class QGridLayout;
class QMenu;
class QRubberBand;

class Resizable_rubber_band : public QWidget {
public:
  Resizable_rubber_band(QWidget* parent = 0);

private:
  QRubberBand* rubberband;
  void resizeEvent(QResizeEvent *);

};

class imageEditor : public QMainWindow
{
    Q_OBJECT

public:
    imageEditor(QWidget *parent = nullptr);
private slots:
    void open();
    void save();
    void autoBinarButton();
    void binarButton();
    void wishColour();
    void change();
    void preview();
    void cut();
    void selectArea();
private:
    void createActions();
    void createMenus();
    void updateList();
    void bandUpdate();

    //Объявление надписей
    QLabel *imageLabel;
    QLabel* previewLabel;
    QLabel* autoLabel;
    QLabel* binarOptionsLabel;
    QLabel* changePaletteLabel;
    QLabel* fColourLabel;
    QLabel* sColourLabel;

    //Объявление кнопок
    QPushButton* autoButton;
    QPushButton* binarizeButton;
    QPushButton* sColourChoose;
    QPushButton* changeColourButton;
    QPushButton* previewButton;

    //Объявление виджета для считывания числовых значений
    QSpinBox* contrastVal;

    //Объявление выпадающих списков
    QComboBox* binarColourButton;
    QComboBox* fColourChoose;

    //Объявление автоматических размещений элементов
    QHBoxLayout* binar;
    QGridLayout* paletteGrid;
    QVBoxLayout* leftLayout;
    QVBoxLayout* rightLayout;
    QHBoxLayout* mainLayout;

    //Определение списков
    QStringList binarColourList;
    QStringList originColours;

    QColor secColor;    
    Resizable_rubber_band* band;

    QWidget* wgt;
    QAction* openAct;
    QAction* saveAct;
    QAction* exitAct;
    QAction* cutAct;
    QAction* selectAreaAct;
    QImage img;
    QMenu* fileMenu;
    QMenu* cutMenu;
};
#endif // IMAGEEDITOR_H

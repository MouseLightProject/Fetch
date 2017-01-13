#pragma once
#include <QtWidgets>
namespace mylib {
#include <array.h>
}
#include "StageScene.h"
#include "imitem.h"
#include "ScaleBar.h"
#include "TilesView.h"
#include "StageView.h"
#include "channelHistogramInformationStruct.h"

namespace fetch {
namespace ui {

class ZoomableView:public QGraphicsView
{
  Q_OBJECT
public:
  ZoomableView(QGraphicsScene *scene, QWidget *parent=0);

  //inline double metersToPixels()                                           {return _scalebar.unit2px();}
  //inline void   setMetersToPixels(double unit2px)                          {_scalebar.setUnit(unit2px);}
  virtual void	wheelEvent(QWheelEvent *event);

  inline void notifyZoomChanged()                                            {emit zoomChanged(transform().m11());}
signals:
  void zoomChanged(double zoom);

protected:
  virtual void drawForeground(QPainter* painter, const QRectF& rect);

private:
  ScaleBar _scalebar;
};

class Figure:public QWidget
{
  Q_OBJECT
public:
  Figure(PlanarStageController *stageController, channelHistogramInformationStruct *channelHistogramInformation,size_t *channelIndex, QWidget *parent=0);//DGA: Added channel histogram information struct pointer and channelIndex pointer arguments
  Figure(double unit2px, QWidget *parent=0);
  virtual ~Figure();

public slots:
  inline void push(mylib::Array *im)                                       {_item->push(im); updatePos(); emit pushed(); maybeFit();}
  inline void imshow(mylib::Array *im, mylib::Array *currentImagePointerAccordingToUI = NULL, bool fromHistogramWidgetUI = false){//DGA: Added currentImagePointerAccordingToUI and fromHistogramWidgetUI boolean as input arguments, respectively set to NULL and false respectively
	  if (!fromHistogramWidgetUI) _previous_im = im; //DGA: If imshow is not being called from histogram widget UI, then the previous image pointer is set to the current image pointer (a new image has been collected)
	  if (!fromHistogramWidgetUI || (fromHistogramWidgetUI && currentImagePointerAccordingToUI == _previous_im) ) {_item->push(im); updatePos(); _item->flip(); maybeFit();} //DGA: If imshow is not being called because of the histogram widget ui (then a new image has been collected), 
  }																																											 //or if it is being called from the UI but the current pointer according to UI equals the previous pointer (then the UI is updating the current image), then the new image should be shown
  inline void fit(void)                                                    {_view->fitInView(_item->mapRectToScene(_item->boundingRect()),Qt::KeepAspectRatio);_view->notifyZoomChanged();}
  inline void fitNext(void)                                                {/*_isFitOnNext=true;*/}
  inline void updatePos(void)                                              {_item->setPos(units::cvt<units::PIXEL_SCALE,PlanarStageController::Unit>(_sc->pos())); _stage->update();}
  inline void updatePos(QPointF r)                                         {_item->setPos(units::cvt<units::PIXEL_SCALE,PlanarStageController::Unit>(r));  _stage->update();}
  inline void autoscale0()                                                 {_item->toggleAutoscale(0); emit autoscaleToggledInFigure(0);}
  inline void autoscale1()                                                 {_item->toggleAutoscale(1); emit autoscaleToggledInFigure(1);}
  inline void autoscale2()                                                 {_item->toggleAutoscale(2); emit autoscaleToggledInFigure(2);}
  inline void autoscale3()                                                 {_item->toggleAutoscale(3); emit autoscaleToggledInFigure(3);}
  inline void fovGeometryChanged(float w_um,float h_um, float radians)     {_item->setFOVGeometry(w_um,h_um,radians);}
  inline void setColormap(const QString& filename)                         {_item->loadColormap(filename);}
  inline void setGamma(float gamma)                                        {_item->setGamma(gamma);}

  inline void fillActive()                                                 {_tv->fillActive();}
  inline void dilateActive()                                               {_tv->dilateActive();}

  //inline void setPixelSizeMicrons(double w, double h)                      {_item->setPixelSizeMicrons(w,h);}
  //inline void setPixelGeometry(double w, double h, double angle)           {_item->setPixelGeometry(w,h,angle);}
         void setDragModeToNoDrag();
         void setDragModeToSelect();
         void setDragModeToPan();
         //void setAddTilesMode();
         //void setDelTilesMode();

signals:
  void lastFigureClosed();
  void pushed();
  void autoscaleToggledInFigure(int);

protected:
  void readSettings();
  void writeSettings();

//  void contextMenuEvent(QContextMenuEvent *event);

private:
  inline void maybeFit()                                                   {if(_isFitOnNext) {fit(); _isFitOnNext=false;}}
         void createActions();
private:
  PlanarStageController *_sc;
  ZoomableView*      _view;
  StageScene         _scene;
  ImItem*            _item;
  StageView*         _stage;
  TilesView*         _tv;

  bool               _isFitOnNext;

  QMenu             *_toolMenu;
  QAction           *_noDragModeAct;
  QAction           *_scrollDragModeAct;
  //QAction           *_addTilesModeAct;
  //QAction           *_delTilesModeAct;
  QAction           *_rubberBandModeAct;

  mylib::Array      *_previous_im; //DGA: The previous image pointer, used to decide whether to update the current figure
};


//Figure* imshow (mylib::Array *im);
//void    imclose(Figure *w=0);

}} //end namespace fetch::ui

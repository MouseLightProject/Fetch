#pragma once


#include <QtWidgets>
#include <QGraphicsSimpleTextItem>
#include <QGLShaderProgram>

namespace mylib {
#include <array.h>
}

#include "channelHistogramInformationStruct.h"
#include "HistogramUtilities.h"

namespace fetch {
namespace ui {

class ImItem: public QGraphicsItem, protected QOpenGLFunctions
{
public:
  ImItem(channelHistogramInformationStruct *_channelHistogramInformation, size_t *channelIndex); //DGA: ImItem now takes in pointers to _channelHistogramInformation and channelIndex
  virtual ~ImItem();

  QRectF boundingRect  () const;                                           // in meters
  void   paint         (QPainter                       *painter,
                        const QStyleOptionGraphicsItem *option,
                        QWidget                        *widget = 0);
  void push(mylib::Array *plane);
  void flip(int isupdate=1);

  void toggleAutoscale(int chan);
  void setRotation(double radians);
  void setFOVGeometry(float w_um, float h_um, float rotation_radians);

  void loadColormap(const QString& filename);
  void setGamma(float gamma)                                              {_gamma=gamma; update();}

  inline double rotationRadians()                                         {return _rotation_radians;}
protected:
  void updateDisplayLists();
  void _common_setup();
  void _loadTex(mylib::Array *im);
  void _setupShader();
  void _updateCmapCtrlPoints();
  void _scaleImage(mylib::Array *data); //DGA: scaleImage function takes in a pointer to data

  void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *e);

  float _fill;
  float _gain;
  float _bias;
  float _gamma;

  QGraphicsSimpleTextItem _text;
  unsigned int _hQuadDisplayList;
  unsigned int _hTexture;
  unsigned int _nchan;                   // updated when an image is pushed
  unsigned int _show_mode;
  unsigned int _pixelValueCounts[65536]; //DGA: Array used to store pixel counts PDF
  bool _determinedMaximumValueForDataType=false; //DGA: Boolean to check if the maximum value for the data type has been determined
  double _maximumValueForImageDataType; //DGA: Double storing the maximum value for the data type

  QGLShaderProgram _shader;
  unsigned int _hShaderPlane;
  unsigned int _hShaderCmap;
  unsigned int _hTexCmapCtrlS;
  unsigned int _hTexCmapCtrlT;
  unsigned int _hTexCmap;

  unsigned int _cmap_ctrl_count;
  unsigned int _cmap_ctrl_last_size;
  float *_cmap_ctrl_s,
        *_cmap_ctrl_t;

  QRectF _bbox_um;
  //QSizeF _pixel_size_meters;
  double _rotation_radians;

  int  _loaded;
  bool _resetscale_next;
  bool _autoscale_next;
  int  _selected_channel;

private:
  channelHistogramInformationStruct *_channelHistogramInformation; //DGA: pointer to channel histogram information struct
  size_t *_channelIndex; //DGA: Pointer to channel index
  GLint _displayChannelsArray[4]; //DGA: Array storing whether or not channels should be displayed
};


}}//end fetch::ui


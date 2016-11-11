#pragma once

#include <QtWidgets>
#include <QtOpenGL>

namespace mylib
{
#include <array.h>
}

#include "ui/StageController.h"

#define GOLDENRATIO 1.61803399

namespace fetch {
namespace ui {


class TilesView : public QGraphicsObject
{
  Q_OBJECT

public:
  TilesView(TilingController *tc, QGraphicsItem* parent = 0);


  ~TilesView();

  QRectF boundingRect() const;
  void   paint         (QPainter                       *painter,
                        const QStyleOptionGraphicsItem *option,
                        QWidget                        *widget = 0);

public slots:
  void addSelection(QPainterPath& path);
  void removeSelection(QPainterPath& path);
  void markSelectedAreaUserReset(QPainterPath& path);
  void markSelectedAreaAsDone(QPainterPath& path);
  void markSelectedAreaAsSafe(QPainterPath& path);
  void markSelectedAreaAsNotSafe(QPainterPath& path);
  void markSelectedAreaAsNotDone(QPainterPath& path);
  void markSelectedAreaAsExplorable(QPainterPath& path);
  void markSelectedAreaAsNotExplorable(QPainterPath& path);

  void update_tiling();
  void show(bool tf);
  void refreshPlane();
  void refreshLatticeAttributes(unsigned itile, unsigned int attr);

  void fillActive();
  void dilateActive();

protected:
  virtual void hoverMoveEvent (QGraphicsSceneHoverEvent *event);
  virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

private:
  typedef TilingController::TTransform TTransform;

  TilingController *tc_;
  QGLBuffer vbo_,ibo_,cbo_;                    //vertex, index, and color  buffer objects for big lattice
  //QGLBUffer cursorColorVBO_,cursorIndexVBO_; //colors and indeces for highlighting tiles near the cursor
  //bool      cursorActive_;                   //
  //int       cursorIndexLength_;
  int           icursor_;
  QRectF        bbox_;
  bool          is_active_;
  QVector<QRgb> color_table_attr2idx_;          // color table for decoding attribute mask
  QVector<QRgb> color_table_idx2rgb_;           // color table defining how attributes determine tile color

  QImage    *latticeImage_;

  void draw_grid_();
  void draw_cursor_();
  void paint_lattice_(const QPainterPath& path, const QColor& pc, const QColor &bc);
  void paint_lattice_attribute_image_();
  void init_cursor_(const QPointF& pos);
  void initIBO();
  void updateIBO();
  void initVBO();
  void updateVBO();
  void initCBO();


  void updateCBO();

  void init_color_tables();
};

}} // end fetch::ui namespace

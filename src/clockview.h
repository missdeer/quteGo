#ifndef CLOCKVIEW_H
#define CLOCKVIEW_H

#include <QGraphicsView>

#include "goboard.h"

class game_state;
class ClockView : public QGraphicsView
{
    Q_OBJECT

    QGraphicsScene    *m_scene {};
    QGraphicsTextItem *m_text {};
signals:
    void clicked();

protected:
    virtual void resizeEvent(QResizeEvent *) override;
    virtual void mousePressEvent(QMouseEvent *e) override;

public:
    ClockView(QWidget *parent);
    void update_font(const QFont &);
    void update_pos();
    void set_text(const QString &s);
    void flash(bool on);
    void set_time(game_state *, stone_color);
};

#endif

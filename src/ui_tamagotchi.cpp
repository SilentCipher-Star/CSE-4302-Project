#include "../include/ui_tamagotchi.hpp"
#include "../include/habit.hpp"
#include "../include/student.hpp"
#include <QTime>
#include <QDate>
#include <QRandomGenerator>
#include <QtMath>
#include <algorithm>

// ═══════════════════════════════════════════════════════════════════════════════
//  TamagotchiCanvas
// ═══════════════════════════════════════════════════════════════════════════════

TamagotchiCanvas::TamagotchiCanvas(QWidget *parent)
    : QWidget(parent), m_animTimer(new QTimer(this))
{
    setMinimumSize(420, 340);
    connect(m_animTimer, &QTimer::timeout, this, &TamagotchiCanvas::onAnimTick);
    m_animTimer->start(80);
}

void TamagotchiCanvas::setState(TamaState state)  { m_state = state;  update(); }
void TamagotchiCanvas::setStats(const TamaStats &s){ m_stats = s;      update(); }
void TamagotchiCanvas::setAccentColor(const QString &a){ m_accent = a; update(); }

void TamagotchiCanvas::onAnimTick()
{
    ++m_frame;
    update();
}

// ─── paintEvent ──────────────────────────────────────────────────────────────
void TamagotchiCanvas::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    drawRoom(p);

    qreal cx = width()  * 0.50;
    qreal cy = height() * 0.72;
    drawCharacter(p, cx, cy, 1.0);
}

// ─── Room ────────────────────────────────────────────────────────────────────
void TamagotchiCanvas::drawRoom(QPainter &p)
{
    int W = width(), H = height();
    int floorY = (int)(H * 0.70);

    // Wall gradient
    QLinearGradient wallGrad(0, 0, 0, floorY);
    wallGrad.setColorAt(0, QColor(232, 228, 248));
    wallGrad.setColorAt(1, QColor(215, 210, 238));
    p.fillRect(0, 0, W, floorY, wallGrad);

    // Floor
    p.fillRect(0, floorY, W, H - floorY, QColor(185, 145, 100));
    p.setPen(QPen(QColor(160, 120, 75), 1));
    for (int x = 0; x < W; x += 55)
        p.drawLine(x, floorY, x, H);
    p.fillRect(0, floorY, W, 5, QColor(140, 100, 60));

    // ── Window (left) ──
    int hour = QTime::currentTime().hour();
    bool night = (hour < 6 || hour >= 20);
    int wx = 28, wy = 30, ww = 105, wh = 135;
    p.setBrush(QColor(200, 185, 150));
    p.setPen(QPen(QColor(160, 135, 100), 3));
    p.drawRect(wx, wy, ww, wh);
    QColor sky = night ? QColor(12, 18, 55) : QColor(130, 195, 240);
    p.fillRect(wx+4, wy+4, ww-8, wh-8, sky);
    p.setPen(QPen(QColor(200, 185, 150), 3));
    p.drawLine(wx+ww/2, wy+4, wx+ww/2, wy+wh-4);
    p.drawLine(wx+4, wy+wh/2, wx+ww-4, wy+wh/2);

    if (night) {
        p.setBrush(QColor(255, 252, 210)); p.setPen(Qt::NoPen);
        p.drawEllipse(wx+18, wy+18, 28, 28);
        p.setBrush(sky); p.drawEllipse(wx+26, wy+14, 24, 24);
        static const int sx[] = {70,82,60,90,68}; static const int sy[] = {14,32,46,50,60};
        p.setBrush(QColor(255,252,210));
        for (int i=0;i<5;i++) p.drawEllipse(wx+sx[i]-2, wy+sy[i]-2, 4, 4);
    } else {
        int sunX = wx+72, sunY = wy+35;
        p.setBrush(QColor(255,222,50)); p.setPen(QPen(QColor(240,190,0),2));
        p.drawEllipse(sunX-16, sunY-16, 32, 32);
        for (int a=0;a<360;a+=45) {
            qreal rad = qDegreesToRadians((qreal)a);
            p.setPen(QPen(QColor(255,200,0),2));
            p.drawLine(sunX+(int)(22*qCos(rad)), sunY+(int)(22*qSin(rad)),
                       sunX+(int)(30*qCos(rad)), sunY+(int)(30*qSin(rad)));
        }
    }

    // ── Bookshelf (center) ──
    int bsx = W/2-65, bsy=25, bsw=130, bsh=195;
    p.setBrush(QColor(160,110,60)); p.setPen(QPen(QColor(120,80,40),2));
    p.drawRect(bsx, bsy, bsw, bsh);
    static const QColor bookCols[] = {
        {210,75,75},{75,145,215},{75,195,115},
        {215,175,55},{175,75,215},{215,125,55},{75,195,195}
    };
    int brain = m_stats.brain;
    for (int shelf=0; shelf<4; shelf++) {
        int shelfY = bsy+10+shelf*46;
        p.fillRect(bsx+4, shelfY+38, bsw-8, 6, QColor(140,95,50));
        int books = qMin(10, (int)((qreal)brain/100.0 * 10.0) - shelf * 2);
        if (books < 0) books = 0;
        for (int b=0; b<books; b++) {
            int bh2 = 20+(b%3)*5;
            p.setBrush(bookCols[(b+shelf*3) % 7]);
            p.setPen(Qt::NoPen);
            p.drawRect(bsx+6+b*12, shelfY+38-bh2, 10, bh2);
        }
    }

    // ── Desk (right) ──
    int dx = W-180, dy = floorY-58, dw = 162, dh = 52;
    p.setBrush(QColor(178,130,72)); p.setPen(QPen(QColor(138,100,52),2));
    p.drawRoundedRect(dx, dy, dw, dh, 4, 4);
    p.setBrush(QColor(152,108,58));
    p.drawRect(dx+10, dy+dh-2, 12, floorY-dy-dh+2);
    p.drawRect(dx+dw-22, dy+dh-2, 12, floorY-dy-dh+2);

    // Laptop
    int lx=dx+18, ly=dy-38;
    p.setBrush(QColor(38,42,56)); p.setPen(QPen(QColor(75,80,98),2));
    p.drawRoundedRect(lx, ly, 82, 52, 4, 4);
    p.setFont(QFont("Monospace",5)); p.setPen(QColor(100,220,100));
    p.drawText(lx+5, ly+14, "int main(){");
    p.setPen(QColor(100,180,255)); p.drawText(lx+8, ly+22, "  study();");
    p.setPen(QColor(255,200,100)); p.drawText(lx+8, ly+30, "  pray();");
    p.setPen(QColor(100,220,100)); p.drawText(lx+5, ly+38, "}");
    p.setBrush(QColor(58,62,76)); p.setPen(QPen(QColor(38,42,56),2));
    p.drawRoundedRect(lx-4, ly+53, 90, 10, 3, 3);

    // ── Whiteboard (left, above floor) ──
    int wbx=30, wby=floorY-115, wbw=125, wbh=75;
    p.setBrush(Qt::white); p.setPen(QPen(QColor(180,188,218),2));
    p.drawRoundedRect(wbx, wby, wbw, wbh, 4, 4);
    p.fillRect(wbx, wby, wbw, 14, QColor(100,120,200));
    p.fillRect(wbx, wby+7, wbw, 7, QColor(100,120,200));
    p.setPen(Qt::white); p.setFont(QFont("Arial",6,QFont::Bold));
    p.drawText(QRect(wbx,wby,wbw,14), Qt::AlignCenter, "Today");

    // Exam alert on whiteboard
    if (m_stats.examSoon && m_stats.daysToExam <= 3) {
        p.setPen(QColor(220,50,50)); p.setFont(QFont("Arial",6,QFont::Bold));
        p.drawText(wbx+5, wby+28, "⚠ " + m_stats.examName.left(15));
        p.setFont(QFont("Arial",6));
        p.drawText(wbx+5, wby+40, QString("in %1 day(s)!").arg(m_stats.daysToExam));
    } else {
        p.setPen(QColor(80,100,160)); p.setFont(QFont("Arial",6));
        p.drawText(wbx+5, wby+30, "No urgent exams");
        p.drawText(wbx+5, wby+44, "Keep studying!");
    }

    // ── Calendar (right of shelf) ──
    int calx = bsx+bsw+15, caly=35, calw=72, calh=78;
    p.setBrush(Qt::white); p.setPen(QPen(QColor(190,55,55),2));
    p.drawRect(calx, caly, calw, calh);
    p.fillRect(calx, caly, calw, 15, QColor(190,55,55));
    p.setPen(Qt::white); p.setFont(QFont("Arial",6,QFont::Bold));
    p.drawText(QRect(calx,caly,calw,15), Qt::AlignCenter,
               QDate::currentDate().toString("MMM yyyy"));
    int today = QDate::currentDate().day();
    p.setFont(QFont("Arial",5));
    for (int d=1; d<=28; d++) {
        int col=(d-1)%7, row=(d-1)/7;
        int gx=calx+4+col*9, gy=caly+20+row*12;
        if (d == today) {
            p.setBrush(QColor(80,140,220)); p.setPen(Qt::NoPen);
            p.drawEllipse(gx-1, gy-7, 9, 9);
            p.setPen(Qt::white);
        } else {
            p.setPen(QColor(80,80,100));
        }
        p.drawText(gx, gy, QString::number(d));
    }
}

// ─── Character ───────────────────────────────────────────────────────────────
void TamagotchiCanvas::drawCharacter(QPainter &p, qreal cx, qreal cy, qreal sc)
{
    qreal bounce = 0, sway = 0;
    switch (m_state) {
    case TamaState::Happy:
        bounce = -qAbs(qSin(m_frame * 0.15)) * 10 * sc;
        break;
    case TamaState::Stressed:
        sway = qSin(m_frame * 0.38) * 7 * sc;
        break;
    case TamaState::Neutral:
        sway = qSin(m_frame * 0.05) * 2.5 * sc;
        break;
    default: break;
    }
    qreal px = cx + sway, py = cy + bounce;

    // ── Sick: lying down ──
    if (m_state == TamaState::Sick) {
        p.setBrush(QColor(195,175,158)); p.setPen(QPen(QColor(155,135,118),2));
        p.drawRoundedRect((int)(px-58), (int)(py-18), 116, 34, 8, 8);
        p.setBrush(QColor(228,200,200));
        p.drawRoundedRect((int)(px-62), (int)(py-26), 124, 24, 10, 10);
        p.setBrush(QColor(195,228,195)); p.setPen(QPen(QColor(148,178,148),2));
        p.drawRoundedRect((int)(px-32), (int)(py-20), 62, 20, 8, 8);
        p.setBrush(QColor(255,222,185)); p.setPen(QPen(QColor(198,170,132),2));
        p.drawEllipse((int)(px+22), (int)(py-30), 26, 26);
        drawSickFace(p, px+35, py-17, 11);
        // Zzz
        int zOff = (m_frame*2) % 60;
        p.setPen(QPen(QColor(148,172,225, 230-zOff*3), 2));
        p.setFont(QFont("Arial", 9, QFont::Bold));
        p.drawText((int)(px+52), (int)(py-18-zOff/2), "zzz");
        return;
    }

    // ── Body ──
    QColor body;
    switch (m_state) {
    case TamaState::Happy:   body = QColor(98,208,118);  break;
    case TamaState::Neutral: body = QColor(98,158,218);  break;
    case TamaState::Tired:   body = QColor(148,158,188); break;
    case TamaState::Stressed:body = QColor(228,128,58);  break;
    default:                 body = QColor(128,178,198); break;
    }
    qreal bw=42*sc, bh=50*sc;
    p.setBrush(body); p.setPen(QPen(body.darker(138),2));
    p.drawRoundedRect((int)(px-bw/2), (int)(py-bh), (int)bw, (int)bh, 11, 11);

    // Arms
    p.setBrush(body);
    if (m_state == TamaState::Happy) {
        qreal wave = qSin(m_frame * 0.2) * 14;
        p.drawEllipse((int)(px-bw/2-11), (int)(py-bh+4+wave), 11, 21);
        p.drawEllipse((int)(px+bw/2),    (int)(py-bh+4-wave), 11, 21);
    } else if (m_state == TamaState::Tired) {
        p.drawEllipse((int)(px-bw/2-10), (int)(py-bh/2+8),  11, 21);
        p.drawEllipse((int)(px+bw/2),    (int)(py-bh/2+8),  11, 21);
    } else {
        p.drawEllipse((int)(px-bw/2-10), (int)(py-bh+13), 11, 21);
        p.drawEllipse((int)(px+bw/2),    (int)(py-bh+13), 11, 21);
    }

    // Legs
    p.drawRoundedRect((int)(px-15), (int)(py-13), 13, 17, 5, 5);
    p.drawRoundedRect((int)(px+2),  (int)(py-13), 13, 17, 5, 5);

    // Neutral: holds book
    if (m_state == TamaState::Neutral) {
        p.setBrush(QColor(178,75,75)); p.setPen(QPen(QColor(138,48,48),2));
        p.drawRoundedRect((int)(px+bw/2+8), (int)(py-bh/2-2), 20, 26, 3, 3);
        p.setPen(QColor(198,118,118)); p.setFont(QFont("Arial",4));
        p.drawText((int)(px+bw/2+10), (int)(py-bh/2+10), "OOP");
    }

    // Stressed: flying papers
    if (m_state == TamaState::Stressed) {
        for (int i=0; i<3; i++) {
            qreal ang = m_frame*0.08 + i*2.09;
            int papx = (int)(px + qCos(ang)*52);
            int papy = (int)(py-bh/2 + qSin(ang*0.7)*28);
            p.save(); p.translate(papx, papy);
            p.rotate(m_frame*3.0 + i*40);
            p.setBrush(Qt::white); p.setPen(QPen(QColor(175,175,175),1));
            p.drawRect(-8,-6,16,12);
            p.setPen(QColor(100,100,180)); p.setFont(QFont("Arial",3));
            p.drawText(-6,-1,"===="); p.drawText(-6,3,"====");
            p.restore();
        }
    }

    // ── Head ──
    qreal hr = 27*sc;
    qreal hcx = px, hcy = py - bh - hr + 8;
    p.setBrush(QColor(255,222,182)); p.setPen(QPen(QColor(198,170,132),2));
    p.drawEllipse((int)(hcx-hr), (int)(hcy-hr), (int)(hr*2), (int)(hr*2));

    // Blush
    if (m_state==TamaState::Happy || m_state==TamaState::Neutral) {
        p.setBrush(QColor(255,158,158,90)); p.setPen(Qt::NoPen);
        p.drawEllipse((int)(hcx-hr+3),(int)(hcy+2),13,9);
        p.drawEllipse((int)(hcx+hr-16),(int)(hcy+2),13,9);
    }

    // Face
    switch (m_state) {
    case TamaState::Happy:   drawHappyFace(p, hcx, hcy, hr);   break;
    case TamaState::Neutral: drawNeutralFace(p, hcx, hcy, hr); break;
    case TamaState::Tired:   drawTiredFace(p, hcx, hcy, hr);   break;
    case TamaState::Stressed:drawStressedFace(p, hcx, hcy, hr);break;
    default: break;
    }

    drawAccessories(p, hcx, hcy, hr);
    drawEffects(p, px, py-bh/2, sc);
}

void TamagotchiCanvas::drawHappyFace(QPainter &p, qreal cx, qreal cy, qreal r)
{
    // Star eyes
    p.setBrush(QColor(58,58,98)); p.setPen(Qt::NoPen);
    p.drawEllipse((int)(cx-r*0.38-5),(int)(cy-r*0.18-5),10,10);
    p.drawEllipse((int)(cx+r*0.38-5),(int)(cy-r*0.18-5),10,10);
    p.setBrush(QColor(255,218,55));
    p.drawEllipse((int)(cx-r*0.38-3),(int)(cy-r*0.18-3),6,6);
    p.drawEllipse((int)(cx+r*0.38-3),(int)(cy-r*0.18-3),6,6);
    // Big smile
    p.setBrush(Qt::NoBrush); p.setPen(QPen(QColor(175,98,98),2));
    p.drawArc((int)(cx-10),(int)(cy+r*0.22-4),20,14,0,-180*16);
}

void TamagotchiCanvas::drawNeutralFace(QPainter &p, qreal cx, qreal cy, qreal r)
{
    p.setBrush(QColor(48,48,78)); p.setPen(Qt::NoPen);
    p.drawEllipse((int)(cx-r*0.36-4),(int)(cy-r*0.18-4),9,9);
    p.drawEllipse((int)(cx+r*0.36-4),(int)(cy-r*0.18-4),9,9);
    p.setBrush(Qt::white);
    p.drawEllipse((int)(cx-r*0.36-2),(int)(cy-r*0.22-2),4,4);
    p.drawEllipse((int)(cx+r*0.36-2),(int)(cy-r*0.22-2),4,4);
    p.setPen(QPen(QColor(175,98,98),2)); p.setBrush(Qt::NoBrush);
    p.drawLine((int)(cx-7),(int)(cy+r*0.28),(int)(cx+7),(int)(cy+r*0.28));
    // Yawn every 120 frames
    if (m_frame % 150 > 138) {
        p.setBrush(QColor(175,98,98)); p.setPen(Qt::NoPen);
        p.drawEllipse((int)(cx-5),(int)(cy+r*0.28-3),10,9);
    }
}

void TamagotchiCanvas::drawTiredFace(QPainter &p, qreal cx, qreal cy, qreal r)
{
    // Half-closed droopy eyes
    p.setBrush(QColor(48,48,78)); p.setPen(Qt::NoPen);
    p.drawEllipse((int)(cx-r*0.36-5),(int)(cy-r*0.18-4),10,10);
    p.drawEllipse((int)(cx+r*0.36-5),(int)(cy-r*0.18-4),10,10);
    p.setBrush(QColor(255,222,182));
    p.drawRect((int)(cx-r*0.36-5),(int)(cy-r*0.18-4),(int)10,(int)6);
    p.drawRect((int)(cx+r*0.36-5),(int)(cy-r*0.18-4),(int)10,(int)6);
    // Frown
    p.setBrush(Qt::NoBrush); p.setPen(QPen(QColor(175,98,98),2));
    p.drawArc((int)(cx-8),(int)(cy+r*0.28),16,10,0,180*16);
}

void TamagotchiCanvas::drawStressedFace(QPainter &p, qreal cx, qreal cy, qreal r)
{
    // Wide eyes
    p.setBrush(Qt::white); p.setPen(QPen(QColor(80,80,80),1));
    p.drawEllipse((int)(cx-r*0.38-6),(int)(cy-r*0.2-6),13,13);
    p.drawEllipse((int)(cx+r*0.38-6),(int)(cy-r*0.2-6),13,13);
    p.setBrush(QColor(38,38,78)); p.setPen(Qt::NoPen);
    p.drawEllipse((int)(cx-r*0.38-3),(int)(cy-r*0.2-3),8,8);
    p.drawEllipse((int)(cx+r*0.38-3),(int)(cy-r*0.2-3),8,8);
    // Wavy mouth
    p.setBrush(Qt::NoBrush); p.setPen(QPen(QColor(175,98,98),2));
    p.drawArc((int)(cx-8),(int)(cy+r*0.26),16,10,0,180*16);
    // Sweat drop
    p.setBrush(QColor(98,178,255)); p.setPen(Qt::NoPen);
    p.drawEllipse((int)(cx+r-6),(int)(cy-r*0.4),7,9);
}

void TamagotchiCanvas::drawSickFace(QPainter &p, qreal cx, qreal cy, qreal r)
{
    // X eyes
    p.setPen(QPen(QColor(80,80,80),2));
    p.drawLine((int)(cx-r*0.4-3),(int)(cy-4),(int)(cx-r*0.4+3),(int)(cy+2));
    p.drawLine((int)(cx-r*0.4+3),(int)(cy-4),(int)(cx-r*0.4-3),(int)(cy+2));
    p.drawLine((int)(cx+r*0.4-3),(int)(cy-4),(int)(cx+r*0.4+3),(int)(cy+2));
    p.drawLine((int)(cx+r*0.4+3),(int)(cy-4),(int)(cx+r*0.4-3),(int)(cy+2));
    // Thermometer
    p.setBrush(QColor(255,78,78)); p.setPen(Qt::NoPen);
    p.drawEllipse((int)(cx-2),(int)(cy+5),6,6);
    p.fillRect((int)(cx),(int)(cy-6),(int)2,(int)10,QColor(255,78,78));
}

void TamagotchiCanvas::drawAccessories(QPainter &p, qreal cx, qreal cy, qreal r)
{
    if (m_stats.brain >= 70) {
        // Graduation cap
        p.setBrush(QColor(38,38,58)); p.setPen(QPen(QColor(18,18,38),1));
        p.drawRect((int)(cx-r-2),(int)(cy-r-12),(int)(r*2+4),10);
        p.drawRect((int)(cx-7),(int)(cy-r-24),(int)14,14);
        p.setPen(QPen(QColor(218,178,0),2));
        p.drawLine((int)(cx+r),(int)(cy-r-7),(int)(cx+r+6),(int)(cy-r+4));
    } else if (m_stats.brain >= 40) {
        // Simple headband
        p.setPen(QPen(QColor(m_accent), 4));
        p.setBrush(Qt::NoBrush);
        p.drawArc((int)(cx-r),(int)(cy-r),(int)(r*2),(int)(r*2),15*16,150*16);
    }
}

void TamagotchiCanvas::drawEffects(QPainter &p, qreal cx, qreal cy, qreal sc)
{
    if (m_state == TamaState::Happy) {
        for (int i=0; i<6; i++) {
            qreal ang = m_frame*0.04 + i*1.047;
            int sx = (int)(cx + qCos(ang)*65);
            int sy = (int)(cy + qSin(ang)*38);
            int sz = 4+(int)(qSin(m_frame*0.12+i)*2);
            p.setBrush(QColor(255,228,55,195)); p.setPen(Qt::NoPen);
            p.drawEllipse(sx-sz/2, sy-sz/2, sz, sz);
            p.setPen(QPen(QColor(255,218,0),1));
            p.drawLine(sx-sz,sy,sx+sz,sy); p.drawLine(sx,sy-sz,sx,sy+sz);
        }
    } else if (m_state == TamaState::Tired) {
        for (int i=0; i<3; i++) {
            int yOff = (m_frame*2 + i*40) % 80;
            int alpha = qMax(0, 240 - yOff*3);
            p.setPen(QPen(QColor(148,168,228,alpha),2));
            p.setFont(QFont("Arial", 8+i*2, QFont::Bold));
            p.drawText((int)(cx+38+i*8), (int)(cy-yOff/2), "z");
        }
    }
}


// ═══════════════════════════════════════════════════════════════════════════════
//  UITamagotchi
// ═══════════════════════════════════════════════════════════════════════════════

UITamagotchi::UITamagotchi(AcadenceManager *manager, const QString &role,
                           int userId, QObject *parent)
    : QObject(parent), m_mgr(manager), m_role(role), m_userId(userId),
      m_decayTimer(new QTimer(this))
{
    buildWidget();
    refreshTamagotchi();
    connect(m_decayTimer, &QTimer::timeout, this, &UITamagotchi::onStatDecayTick);
    m_decayTimer->start(30000); // decay every 30 s
}

// ─── Widget layout ───────────────────────────────────────────────────────────
void UITamagotchi::buildWidget()
{
    m_container = new QWidget();
    QHBoxLayout *root = new QHBoxLayout(m_container);
    root->setContentsMargins(10, 10, 10, 10);
    root->setSpacing(14);

    // Canvas
    m_canvas = new TamagotchiCanvas();
    m_canvas->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    root->addWidget(m_canvas, 3);

    // Right panel
    QWidget *panel = new QWidget();
    panel->setFixedWidth(200);
    QVBoxLayout *pv = new QVBoxLayout(panel);
    pv->setSpacing(8);

    QLabel *title = new QLabel("Campus Tamagotchi");
    title->setStyleSheet("font-size:14px; font-weight:bold;");
    title->setAlignment(Qt::AlignCenter);
    pv->addWidget(title);

    m_lblState = new QLabel("State: Neutral");
    m_lblState->setAlignment(Qt::AlignCenter);
    m_lblState->setStyleSheet("font-size:12px; font-weight:bold; padding:4px;");
    pv->addWidget(m_lblState);

    // Stat bars
    auto makeBar = [&](const QString &label, const QString &color) -> QProgressBar* {
        QLabel *lbl = new QLabel(label);
        lbl->setStyleSheet("font-size:11px;");
        pv->addWidget(lbl);
        QProgressBar *bar = new QProgressBar();
        bar->setRange(0,100);
        bar->setFixedHeight(12);
        bar->setTextVisible(false);
        bar->setStyleSheet(pbStyle(color));
        pv->addWidget(bar);
        return bar;
    };

    m_pbHealth = makeBar("Health 💪", "#e05555");
    m_pbMood   = makeBar("Mood 😊",   "#9b59b6");
    m_pbEnergy = makeBar("Energy ⚡", "#e8a020");
    m_pbBrain  = makeBar("Brain 🧠",  "#3498db");

    pv->addSpacing(6);

    m_lblMessage = new QLabel("");
    m_lblMessage->setWordWrap(true);
    m_lblMessage->setAlignment(Qt::AlignCenter);
    m_lblMessage->setStyleSheet(
        "font-size:11px; color:#555; background:#f5f5f8;"
        "border-radius:6px; padding:6px;");
    m_lblMessage->setMinimumHeight(55);
    pv->addWidget(m_lblMessage);

    pv->addStretch();

    // Buttons
    auto makeBtn = [&](const QString &text, const QString &color) -> QPushButton* {
        QPushButton *btn = new QPushButton(text);
        btn->setFixedHeight(34);
        btn->setStyleSheet(
            QString("QPushButton { background:%1; color:white; border-radius:6px; font-size:12px; }"
                    "QPushButton:hover { background:%2; }").arg(color, QColor(color).darker(115).name()));
        return btn;
    };

    m_btnFeed  = makeBtn("Feed 🍱",  "#27ae60");
    m_btnStudy = makeBtn("Study 📚", "#2980b9");
    m_btnSleep = makeBtn("Sleep 😴", "#8e44ad");

    pv->addWidget(m_btnFeed);
    pv->addWidget(m_btnStudy);
    pv->addWidget(m_btnSleep);

    root->addWidget(panel, 0);

    connect(m_btnFeed,  &QPushButton::clicked, this, &UITamagotchi::onFeedClicked);
    connect(m_btnStudy, &QPushButton::clicked, this, &UITamagotchi::onStudyClicked);
    connect(m_btnSleep, &QPushButton::clicked, this, &UITamagotchi::onSleepClicked);
}

// ─── Stat computation ────────────────────────────────────────────────────────
TamaStats UITamagotchi::computeStats()
{
    TamaStats s;
    s.energy = m_energy;

    // Brain: CGPA
    Student *stu = m_mgr->getStudent(m_userId);
    if (stu) {
        s.brain = (int)(stu->getCGPA() / 4.0 * 100.0);
        delete stu;
    }

    // Health: habit streaks + completion
    QVector<Habit*> habits = m_mgr->getHabits(m_userId);
    if (!habits.isEmpty()) {
        int totalStreak = 0, completed = 0;
        for (auto h : habits) {
            totalStreak += h->streak;
            if (h->isCompleted) completed++;
        }
        double rate = (double)completed / habits.size();
        double avgS = qMin((double)totalStreak / habits.size() / 7.0, 1.0);
        s.health = (int)(rate * 55 + avgS * 45);
        qDeleteAll(habits);
    }

    // Mood: prayers today
    DailyPrayerStatus prayers = m_mgr->getDailyPrayers(
        m_userId, QDate::currentDate().toString("yyyy-MM-dd"));
    int done = (prayers.getFajr()?1:0) + (prayers.getDhuhr()?1:0) +
               (prayers.getAsr()?1:0) + (prayers.getMaghrib()?1:0) +
               (prayers.getIsha()?1:0);
    s.mood = done * 20;

    // Upcoming exam
    QVector<Assessment> all = m_mgr->getStudentAssessments(m_userId);
    QDate today = QDate::currentDate();
    s.daysToExam = 999;
    for (const auto &a : all) {
        QDate d = QDate::fromString(a.getDate(), "yyyy-MM-dd");
        if (!d.isValid()) continue;
        int dl = today.daysTo(d);
        if (dl >= 0 && dl < s.daysToExam) {
            s.daysToExam = dl;
            s.examName   = a.getTitle() + " (" + a.getCourseName() + ")";
        }
    }
    s.examSoon = (s.daysToExam <= 5);

    return s;
}

TamaState UITamagotchi::stateFromStats(const TamaStats &s)
{
    // Sick: habits and attendance both bad
    QVector<AttendanceRecord> att = m_mgr->getStudentAttendance(m_userId);
    bool critAtt = false;
    for (const auto &r : att) {
        double pct = r.getTotalClasses() > 0
            ? (double)r.getAttendedClasses() / r.getTotalClasses() * 100.0 : 100.0;
        if (pct < 60.0) { critAtt = true; break; }
    }
    if (critAtt && s.health < 35) return TamaState::Sick;

    if (s.examSoon && s.daysToExam <= 3) return TamaState::Stressed;
    if (s.mood < 25 || s.energy < 20)    return TamaState::Tired;

    int avg = (s.health + s.mood + s.energy + s.brain) / 4;
    if (avg >= 65) return TamaState::Happy;
    return TamaState::Neutral;
}

QString UITamagotchi::messageForState(TamaState state, const TamaStats &s)
{
    static const QStringList happy   = {"Let's ace this semester! ✨","GPA going up! 📈","Prayers done, feeling great!","Streak maintained! 💪"};
    static const QStringList neutral = {"Time to hit the books...","What's for dinner? 🍜","Chapter 5 looks tough.","Need coffee... ☕"};
    static const QStringList tired   = {"Just 5 more minutes... 😴","Why so many assignments?","Please log a habit...","So sleepy..."};
    static const QStringList stressed= {"EXAM SOON!! 😰","I haven't studied enough!","Why did I procrastinate?!","Need to review notes!!"};
    static const QStringList sick    = {"Please attend classes... 🤒","Low attendance is bad...","Do your habits please!","I need rest..."};

    const QStringList *pool = &neutral;
    switch (state) {
    case TamaState::Happy:   pool = &happy;    break;
    case TamaState::Tired:   pool = &tired;    break;
    case TamaState::Stressed:pool = &stressed; break;
    case TamaState::Sick:    pool = &sick;     break;
    default: break;
    }

    if (state == TamaState::Stressed && !s.examName.isEmpty())
        return QString("\"⚠ %1\nin %2 day(s)!\"").arg(s.examName.left(30)).arg(s.daysToExam);

    int idx = QRandomGenerator::global()->bounded(pool->size());
    return QString("\"%1\"").arg((*pool)[idx]);
}

// ─── Apply stats to UI ───────────────────────────────────────────────────────
void UITamagotchi::applyStats()
{
    m_pbHealth->setValue(m_stats.health);
    m_pbMood->setValue(m_stats.mood);
    m_pbEnergy->setValue(m_stats.energy);
    m_pbBrain->setValue(m_stats.brain);

    TamaState state = stateFromStats(m_stats);
    m_canvas->setState(state);
    m_canvas->setStats(m_stats);

    static const char* names[] = {"Happy 😊","Neutral 😐","Tired 😴","Stressed 😰","Sick 🤒"};
    static const char* colors[] = {"#27ae60","#2980b9","#7f8c8d","#e67e22","#c0392b"};
    int idx = (int)state;
    m_lblState->setText(names[idx]);
    m_lblState->setStyleSheet(
        QString("font-size:12px; font-weight:bold; color:white; background:%1;"
                "border-radius:5px; padding:4px;").arg(colors[idx]));

    m_lblMessage->setText(messageForState(state, m_stats));

    int h = QTime::currentTime().hour();
    m_btnSleep->setEnabled(h >= 22 || h < 4);
}

// ─── refresh ─────────────────────────────────────────────────────────────────
void UITamagotchi::refreshTamagotchi()
{
    m_stats = computeStats();
    applyStats();
}

// ─── Slots ───────────────────────────────────────────────────────────────────
void UITamagotchi::onFeedClicked()
{
    m_stats.mood   = qMin(100, m_stats.mood   + 15);
    m_stats.health = qMin(100, m_stats.health + 10);
    applyStats();
}

void UITamagotchi::onStudyClicked()
{
    m_energy = qMin(100, m_energy + 20);
    m_stats.energy = m_energy;
    m_stats.brain  = qMin(100, m_stats.brain + 5);
    applyStats();
}

void UITamagotchi::onSleepClicked()
{
    m_energy = qMin(100, m_energy + 40);
    m_stats.energy = m_energy;
    m_stats.health = qMin(100, m_stats.health + 20);
    applyStats();
}

void UITamagotchi::onStatDecayTick()
{
    m_energy = qMax(0, m_energy - 3);
    m_stats   = computeStats();
    applyStats();
}

// ─── helpers ─────────────────────────────────────────────────────────────────
QString UITamagotchi::pbStyle(const QString &color)
{
    return QString(
        "QProgressBar { background:#e0e0e8; border-radius:4px; }"
        "QProgressBar::chunk { background:%1; border-radius:4px; }").arg(color);
}

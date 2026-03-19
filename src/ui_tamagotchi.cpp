#include "../include/ui_tamagotchi.hpp"
#include "../include/theme.hpp"
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
    setMinimumSize(240, 180);
    connect(m_animTimer, &QTimer::timeout, this, &TamagotchiCanvas::onAnimTick);
    m_animTimer->start(80);
}

void TamagotchiCanvas::setState(TamaState state)
{
    m_state = state;
    update();
}
void TamagotchiCanvas::setStats(const TamaStats &s)
{
    m_stats = s;
    update();
}
void TamagotchiCanvas::setAccentColor(const QString &a)
{
    m_accent = a;
    update();
}

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
    p.setRenderHint(QPainter::TextAntialiasing);

    // Fixed logical resolution for perfect scaling
    const qreal V_WIDTH = 1280.0;
    const qreal V_HEIGHT = 720.0;

    qreal scale = qMin(width() / V_WIDTH, height() / V_HEIGHT);
    p.translate((width() - V_WIDTH * scale) / 2.0, (height() - V_HEIGHT * scale) / 2.0);
    p.scale(scale, scale);

    drawRoom(p);
    drawCharacter(p, V_WIDTH * 0.50, V_HEIGHT * 0.85, 1.0);
}

// ─── Room ────────────────────────────────────────────────────────────────────
void TamagotchiCanvas::drawRoom(QPainter &p)
{
    int W = 1280, H = 720;
    int floorY = 560; // 720 * 0.77

    // Wall gradient
    QLinearGradient wallGrad(0, 0, 0, floorY);
    wallGrad.setColorAt(0, QColor(240, 242, 250));
    wallGrad.setColorAt(1, QColor(220, 225, 240));
    p.fillRect(0, 0, W, floorY, wallGrad);

    // Floor
    p.fillRect(0, floorY, W, H - floorY, QColor(190, 160, 120));
    p.setPen(QPen(QColor(170, 135, 95), 2));
    for (int x = 0; x < W; x += 100)
        p.drawLine(x, floorY, x, H);
    p.fillRect(0, floorY, W, 12, QColor(140, 100, 60)); // Baseboard

    // ── Window (left) ──
    int hour = QTime::currentTime().hour();
    bool night = (hour < 6 || hour >= 20);
    int wx = 100, wy = 100, ww = 200, wh = 240;
    p.setBrush(QColor(210, 195, 160));
    p.setPen(QPen(QColor(170, 145, 110), 4));
    p.drawRect(wx, wy, ww, wh);
    QColor sky = night ? QColor(15, 20, 60) : QColor(130, 200, 250);
    p.fillRect(wx + 4, wy + 4, ww - 8, wh - 8, sky);
    p.setPen(QPen(QColor(210, 195, 160), 4));
    p.drawLine(wx + ww / 2, wy + 4, wx + ww / 2, wy + wh - 4);
    p.drawLine(wx + 4, wy + wh / 2, wx + ww - 4, wy + wh / 2);

    if (night)
    {
        p.setBrush(QColor(255, 252, 210));
        p.setPen(Qt::NoPen);
        p.drawEllipse(wx + 35, wy + 35, 60, 60);
        p.setBrush(sky);
        p.drawEllipse(wx + 50, wy + 25, 50, 50);
        static const int sx[] = {130, 160, 100, 170, 120};
        static const int sy[] = {40, 70, 100, 110, 130};
        p.setBrush(QColor(255, 252, 210));
        for (int i = 0; i < 5; i++)
            p.drawEllipse(wx + sx[i] - 3, wy + sy[i] - 3, 7, 7);
    }
    else
    {
        int sunX = wx + 140, sunY = wy + 70;
        p.setBrush(QColor(255, 225, 60));
        p.setPen(QPen(QColor(240, 200, 0), 2));
        p.drawEllipse(sunX - 30, sunY - 30, 60, 60);
        for (int a = 0; a < 360; a += 45)
        {
            qreal rad = qDegreesToRadians((qreal)a);
            p.setPen(QPen(QColor(255, 205, 0), 3));
            p.drawLine(sunX + (int)(45 * qCos(rad)), sunY + (int)(45 * qSin(rad)),
                       sunX + (int)(60 * qCos(rad)), sunY + (int)(60 * qSin(rad)));
        }
    }

    // ── Bookshelf (center) ──
    int bsx = 420, bsy = 120, bsw = 220, bsh = 440;
    p.setBrush(QColor(160, 110, 60));
    p.setPen(QPen(QColor(120, 80, 40), 3));
    p.drawRect(bsx, bsy, bsw, bsh);
    static const QColor bookCols[] = {
        {220, 80, 80}, {80, 150, 220}, {80, 200, 120}, {220, 180, 60}, {180, 80, 220}, {220, 130, 60}, {80, 200, 200}};
    int brain = m_stats.brain;
    for (int shelf = 0; shelf < 4; shelf++)
    {
        int shelfY = bsy + 20 + shelf * 100;
        p.fillRect(bsx + 6, shelfY + 85, bsw - 12, 12, QColor(140, 95, 50));
        int books = qMin(10, (int)((qreal)brain / 100.0 * 10.0) - shelf * 2);
        if (books < 0)
            books = 0;
        for (int b = 0; b < books; b++)
        {
            int bh2 = 40 + (b % 3) * 8;
            p.setBrush(bookCols[(b + shelf * 3) % 7]);
            p.setPen(Qt::NoPen);
            p.drawRect(bsx + 12 + b * 18, shelfY + 85 - bh2, 16, bh2);
        }
    }

    // ── Desk (right) ──
    int dx = 880, dy = floorY - 100, dw = 280, dh = 90; // Relative to new floor
    p.setBrush(QColor(185, 135, 80));
    p.setPen(QPen(QColor(145, 105, 60), 3));
    p.drawRoundedRect(dx, dy, dw, dh, 6, 6);
    p.setBrush(QColor(160, 115, 65));
    p.drawRect(dx + 20, dy + dh - 2, 20, floorY - dy - dh + 2);
    p.drawRect(dx + dw - 40, dy + dh - 2, 20, floorY - dy - dh + 2);

    // Laptop
    int lx = dx + 60, ly = dy - 110;
    p.setBrush(QColor(40, 45, 60));
    p.setPen(QPen(QColor(80, 85, 105), 3));
    p.drawRoundedRect(lx, ly, 160, 110, 8, 8);
    p.setFont(QFont(AppFonts::Family, 12, QFont::Bold));
    p.setPen(QColor(120, 230, 120));
    p.drawText(lx + 15, ly + 30, "int main() {");
    p.setPen(QColor(120, 190, 255));
    p.drawText(lx + 25, ly + 55, "study();");
    p.setPen(QColor(255, 210, 120));
    p.drawText(lx + 25, ly + 80, "pray();");
    p.setPen(QColor(120, 230, 120));
    p.drawText(lx + 15, ly + 105, "}");
    p.setBrush(QColor(60, 65, 80));
    p.setPen(QPen(QColor(40, 45, 60), 2));
    p.drawRoundedRect(lx - 10, ly + 112, 180, 16, 6, 6);

    // ── Whiteboard (left, above floor) ──
    int wbx = 80, wby = 380, wbw = 240, wbh = 150;
    p.setBrush(Qt::white);
    p.setPen(QPen(QColor(180, 190, 220), 3));
    p.drawRoundedRect(wbx, wby, wbw, wbh, 5, 5);
    p.fillRect(wbx, wby, wbw, 35, QColor(100, 120, 200));
    p.fillRect(wbx, wby + 17, wbw, 18, QColor(100, 120, 200));
    p.setPen(Qt::white);
    p.setFont(QFont(AppFonts::Family, 14, QFont::Bold));
    p.drawText(QRect(wbx, wby, wbw, 35), Qt::AlignCenter, "Focus Board");

    // Exam alert on whiteboard
    if (m_stats.examSoon && m_stats.daysToExam <= 3)
    {
        p.setPen(QColor(220, 50, 50));
        p.setFont(QFont(AppFonts::Family, 16, QFont::Bold));
        p.drawText(QRect(wbx + 12, wby + 50, wbw - 24, 35), Qt::AlignLeft | Qt::AlignVCenter, "⚠ " + m_stats.examName.left(12) + "..");
        p.setFont(QFont(AppFonts::Family, 14, QFont::Medium));
        p.drawText(QRect(wbx + 12, wby + 100, wbw - 24, 35), Qt::AlignLeft | Qt::AlignVCenter, QString("in %1 day(s)!").arg(m_stats.daysToExam));
    }
    else
    {
        p.setPen(QColor(80, 100, 160));
        p.setFont(QFont(AppFonts::Family, 16, QFont::Bold));
        p.drawText(QRect(wbx, wby + 55, wbw, 35), Qt::AlignCenter, "No urgent exams");
        p.setFont(QFont(AppFonts::Family, 14, QFont::Medium));
        p.drawText(QRect(wbx, wby + 100, wbw, 35), Qt::AlignCenter, "Keep studying!");
    }

    // ── Calendar (right of shelf) ──
    int calx = bsx + bsw + 50, caly = 100, calw = 160, calh = 190;
    p.setBrush(Qt::white);
    p.setPen(QPen(QColor(200, 60, 60), 3));
    p.drawRoundedRect(calx, caly, calw, calh, 4, 4);
    p.fillRect(calx, caly, calw, 35, QColor(200, 60, 60));
    p.setPen(Qt::white);
    p.setFont(QFont(AppFonts::Family, 14, QFont::Bold));
    p.drawText(QRect(calx, caly, calw, 35), Qt::AlignCenter, QDate::currentDate().toString("MMM yyyy"));
    int today = QDate::currentDate().day();
    p.setFont(QFont(AppFonts::Family, 11, QFont::Medium));
    for (int d = 1; d <= 28; d++)
    {
        int col = (d - 1) % 7, row = (d - 1) / 7;
        int gx = calx + 12 + col * 20, gy = caly + 65 + row * 28;
        if (d == today)
        {
            p.setBrush(QColor(80, 140, 220));
            p.setPen(Qt::NoPen);
            p.drawEllipse(gx - 5, gy - 16, 24, 24);
            p.setPen(Qt::white);
        }
        else
        {
            p.setPen(QColor(80, 80, 100));
        }
        p.drawText(gx, gy, QString::number(d));
    }
}

// ─── Character ───────────────────────────────────────────────────────────────
void TamagotchiCanvas::drawCharacter(QPainter &p, qreal cx, qreal cy, qreal sc)
{
    qreal bounce = 0, sway = 0;
    switch (m_state)
    {
    case TamaState::Happy:
        bounce = -qAbs(qSin(m_frame * 0.15)) * 15;
        break;
    case TamaState::Stressed:
        sway = qSin(m_frame * 0.38) * 10;
        break;
    case TamaState::Neutral:
        sway = qSin(m_frame * 0.05) * 4;
        break;
    default:
        break;
    }
    qreal px = cx + sway, py = cy + bounce;

    // ── Sick: lying down ──
    if (m_state == TamaState::Sick)
    {
        p.setBrush(QColor(195, 175, 158));
        p.setPen(QPen(QColor(155, 135, 118), 3));
        p.drawRoundedRect((int)(px - 110), (int)(py - 33), 220, 66, 15, 15);
        p.setBrush(QColor(228, 200, 200));
        p.drawRoundedRect((int)(px - 120), (int)(py - 48), 240, 48, 18, 18);
        p.setBrush(QColor(195, 228, 195));
        p.setPen(QPen(QColor(148, 178, 148), 3));
        p.drawRoundedRect((int)(px - 60), (int)(py - 38), 120, 39, 15, 15);
        p.setBrush(QColor(255, 222, 185));
        p.setPen(QPen(QColor(198, 170, 132), 3));
        p.drawEllipse((int)(px + 45), (int)(py - 60), 51, 51);
        drawSickFace(p, px + 70, py - 35, 21);
        // Zzz
        int zOff = (m_frame * 2) % 60;
        p.setPen(QPen(QColor(148, 172, 225, 230 - zOff * 3), 2));
        p.setFont(QFont(AppFonts::Family, 20, QFont::Bold));
        p.drawText((int)(px + 110), (int)(py - 38 - zOff / 2), "Zzz");
        return;
    }

    // ── Body ──
    QColor body;
    switch (m_state)
    {
    case TamaState::Happy:
        body = QColor(98, 208, 118);
        break;
    case TamaState::Neutral:
        body = QColor(98, 158, 218);
        break;
    case TamaState::Tired:
        body = QColor(148, 158, 188);
        break;
    case TamaState::Stressed:
        body = QColor(228, 128, 58);
        break;
    default:
        body = QColor(128, 178, 198);
        break;
    }
    qreal bw = 64, bh = 75;
    p.setBrush(body);
    p.setPen(QPen(body.darker(138), 2));
    p.drawRoundedRect((int)(px - bw / 2), (int)(py - bh), (int)bw, (int)bh, 16, 16);

    // Arms
    p.setBrush(body);
    if (m_state == TamaState::Happy)
    {
        qreal wave = qSin(m_frame * 0.2) * 27;
        p.drawEllipse((int)(px - bw / 2 - 21), (int)(py - bh + 6 + wave), 21, 39);
        p.drawEllipse((int)(px + bw / 2), (int)(py - bh + 6 - wave), 21, 39);
    }
    else if (m_state == TamaState::Tired)
    {
        p.drawEllipse((int)(px - bw / 2 - 20), (int)(py - bh / 2 + 12), 21, 39);
        p.drawEllipse((int)(px + bw / 2 - 1), (int)(py - bh / 2 + 12), 21, 39);
    }
    else
    {
        p.drawEllipse((int)(px - bw / 2 - 20), (int)(py - bh + 24), 21, 39);
        p.drawEllipse((int)(px + bw / 2 - 1), (int)(py - bh + 24), 21, 39);
    }

    // Legs
    p.drawRoundedRect((int)(px - 27), (int)(py - 22), 22, 30, 9, 9);
    p.drawRoundedRect((int)(px + 5), (int)(py - 22), 22, 30, 9, 9);

    // Neutral: holds book
    if (m_state == TamaState::Neutral)
    {
        p.setBrush(QColor(178, 75, 75));
        p.setPen(QPen(QColor(138, 48, 48), 3));
        p.drawRoundedRect((int)(px + bw / 2 + 15), (int)(py - bh / 2 - 6), 39, 51, 6, 6);
        p.setPen(QColor(230, 180, 180));
        p.setFont(QFont(AppFonts::Family, 10, QFont::Bold));
        p.drawText((int)(px + bw / 2 + 20), (int)(py - bh / 2 + 21), "OOP");
    }

    // Stressed: flying papers
    if (m_state == TamaState::Stressed)
    {
        for (int i = 0; i < 3; i++)
        {
            qreal ang = m_frame * 0.08 + i * 2.09;
            int papx = (int)(px + qCos(ang) * 95);
            int papy = (int)(py - bh / 2 + qSin(ang * 0.7) * 50);
            p.save();
            p.translate(papx, papy);
            p.rotate(m_frame * 3.0 + i * 40);
            p.setBrush(Qt::white);
            p.setPen(QPen(QColor(175, 175, 175), 2));
            p.drawRect(-15, -12, 30, 24);
            p.setPen(QColor(100, 100, 180));
            p.setFont(QFont(AppFonts::Family, 9));
            p.drawText(-12, -3, "===");
            p.drawText(-12, 8, "===");
            p.restore();
        }
    }

    // ── Head ──
    qreal hr = 40;
    qreal hcx = px, hcy = py - bh - hr + 15;
    p.setBrush(QColor(255, 222, 182));
    p.setPen(QPen(QColor(198, 170, 132), 3));
    p.drawEllipse((int)(hcx - hr), (int)(hcy - hr), (int)(hr * 2), (int)(hr * 2));

    // Blush
    if (m_state == TamaState::Happy || m_state == TamaState::Neutral)
    {
        p.setBrush(QColor(255, 158, 158, 100));
        p.setPen(Qt::NoPen);
        p.drawEllipse((int)(hcx - hr + 6), (int)(hcy + 3), 24, 16);
        p.drawEllipse((int)(hcx + hr - 30), (int)(hcy + 3), 24, 16);
    }

    // Face
    switch (m_state)
    {
    case TamaState::Happy:
        drawHappyFace(p, hcx, hcy, hr);
        break;
    case TamaState::Neutral:
        drawNeutralFace(p, hcx, hcy, hr);
        break;
    case TamaState::Tired:
        drawTiredFace(p, hcx, hcy, hr);
        break;
    case TamaState::Stressed:
        drawStressedFace(p, hcx, hcy, hr);
        break;
    default:
        break;
    }

    drawAccessories(p, hcx, hcy, hr);
    drawEffects(p, px, py - bh / 2, sc);
}

void TamagotchiCanvas::drawHappyFace(QPainter &p, qreal cx, qreal cy, qreal r)
{
    // Star eyes
    p.setBrush(QColor(58, 58, 98));
    p.setPen(Qt::NoPen);
    p.drawEllipse((int)(cx - r * 0.38 - 9), (int)(cy - r * 0.18 - 9), 18, 18);
    p.drawEllipse((int)(cx + r * 0.38 - 9), (int)(cy - r * 0.18 - 9), 18, 18);
    p.setBrush(QColor(255, 218, 55));
    p.drawEllipse((int)(cx - r * 0.38 - 6), (int)(cy - r * 0.18 - 6), 12, 12);
    p.drawEllipse((int)(cx + r * 0.38 - 6), (int)(cy - r * 0.18 - 6), 12, 12);
    // Big smile
    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(QColor(175, 98, 98), 3));
    p.drawArc((int)(cx - 18), (int)(cy + r * 0.22 - 7), 36, 27, 0, -180 * 16);
}

void TamagotchiCanvas::drawNeutralFace(QPainter &p, qreal cx, qreal cy, qreal r)
{
    p.setBrush(QColor(48, 48, 78));
    p.setPen(Qt::NoPen);
    p.drawEllipse((int)(cx - r * 0.36 - 7), (int)(cy - r * 0.18 - 7), 15, 15);
    p.drawEllipse((int)(cx + r * 0.36 - 7), (int)(cy - r * 0.18 - 7), 15, 15);
    p.setBrush(Qt::white);
    p.drawEllipse((int)(cx - r * 0.36 - 4), (int)(cy - r * 0.22 - 4), 7, 7);
    p.drawEllipse((int)(cx + r * 0.36 - 4), (int)(cy - r * 0.22 - 4), 7, 7);
    p.setPen(QPen(QColor(175, 98, 98), 3));
    p.setBrush(Qt::NoBrush);
    p.drawLine((int)(cx - 12), (int)(cy + r * 0.28), (int)(cx + 12), (int)(cy + r * 0.28));
    // Yawn every 120 frames
    if (m_frame % 150 > 138)
    {
        p.setBrush(QColor(175, 98, 98));
        p.setPen(Qt::NoPen);
        p.drawEllipse((int)(cx - 9), (int)(cy + r * 0.28 - 6), 18, 16);
    }
}

void TamagotchiCanvas::drawTiredFace(QPainter &p, qreal cx, qreal cy, qreal r)
{
    // Half-closed droopy eyes
    p.setBrush(QColor(48, 48, 78));
    p.setPen(Qt::NoPen);
    p.drawEllipse((int)(cx - r * 0.36 - 9), (int)(cy - r * 0.18 - 7), 18, 18);
    p.drawEllipse((int)(cx + r * 0.36 - 9), (int)(cy - r * 0.18 - 7), 18, 18);
    p.setBrush(QColor(255, 222, 182));
    p.drawRect((int)(cx - r * 0.36 - 9), (int)(cy - r * 0.18 - 7), 18, 10);
    p.drawRect((int)(cx + r * 0.36 - 9), (int)(cy - r * 0.18 - 7), 18, 10);
    // Frown
    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(QColor(175, 98, 98), 3));
    p.drawArc((int)(cx - 15), (int)(cy + r * 0.28), 30, 18, 0, 180 * 16);
}

void TamagotchiCanvas::drawStressedFace(QPainter &p, qreal cx, qreal cy, qreal r)
{
    // Wide eyes
    p.setBrush(Qt::white);
    p.setPen(QPen(QColor(80, 80, 80), 2));
    p.drawEllipse((int)(cx - r * 0.38 - 12), (int)(cy - r * 0.2 - 12), 24, 24);
    p.drawEllipse((int)(cx + r * 0.38 - 12), (int)(cy - r * 0.2 - 12), 24, 24);
    p.setBrush(QColor(38, 38, 78));
    p.setPen(Qt::NoPen);
    p.drawEllipse((int)(cx - r * 0.38 - 6), (int)(cy - r * 0.2 - 6), 13, 13);
    p.drawEllipse((int)(cx + r * 0.38 - 6), (int)(cy - r * 0.2 - 6), 13, 13);
    // Wavy mouth
    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(QColor(175, 98, 98), 3));
    p.drawArc((int)(cx - 15), (int)(cy + r * 0.26), 30, 18, 0, 180 * 16);
    // Sweat drop
    p.setBrush(QColor(98, 178, 255));
    p.setPen(Qt::NoPen);
    p.drawEllipse((int)(cx + r - 12), (int)(cy - r * 0.4), 13, 18);
}

void TamagotchiCanvas::drawSickFace(QPainter &p, qreal cx, qreal cy, qreal r)
{
    // X eyes
    p.setPen(QPen(QColor(80, 80, 80), 3));
    p.drawLine((int)(cx - r * 0.4 - 6), (int)(cy - 7), (int)(cx - r * 0.4 + 6), (int)(cy + 4));
    p.drawLine((int)(cx - r * 0.4 + 6), (int)(cy - 7), (int)(cx - r * 0.4 - 6), (int)(cy + 4));
    p.drawLine((int)(cx + r * 0.4 - 6), (int)(cy - 7), (int)(cx + r * 0.4 + 6), (int)(cy + 4));
    p.drawLine((int)(cx + r * 0.4 + 6), (int)(cy - 7), (int)(cx + r * 0.4 - 6), (int)(cy + 4));
    // Thermometer
    p.setBrush(QColor(255, 78, 78));
    p.setPen(Qt::NoPen);
    p.drawEllipse((int)(cx - 4), (int)(cy + 9), 12, 12);
    p.fillRect((int)(cx), (int)(cy - 12), 4, 21, QColor(255, 78, 78));
}

void TamagotchiCanvas::drawAccessories(QPainter &p, qreal cx, qreal cy, qreal r)
{
    if (m_stats.brain >= 70)
    {
        // Graduation cap
        p.setBrush(QColor(38, 38, 58));
        p.setPen(QPen(QColor(18, 18, 38), 2));
        p.drawRect((int)(cx - r - 6), (int)(cy - r - 22), (int)(r * 2 + 12), 18);
        p.drawRect((int)(cx - 13), (int)(cy - r - 45), 27, 27);
        p.setPen(QPen(QColor(218, 178, 0), 3));
        p.drawLine((int)(cx + r), (int)(cy - r - 13), (int)(cx + r + 12), (int)(cy - r + 7));
    }
    else if (m_stats.brain >= 40)
    {
        // Simple headband
        p.setPen(QPen(QColor(m_accent), 5));
        p.setBrush(Qt::NoBrush);
        p.drawArc((int)(cx - r), (int)(cy - r), (int)(r * 2), (int)(r * 2), 15 * 16, 150 * 16);
    }
}

void TamagotchiCanvas::drawEffects(QPainter &p, qreal cx, qreal cy, qreal sc)
{
    if (m_state == TamaState::Happy)
    {
        for (int i = 0; i < 6; i++)
        {
            qreal ang = m_frame * 0.04 + i * 1.047;
            int sx = (int)(cx + qCos(ang) * 95);
            int sy = (int)(cy + qSin(ang) * 57);
            int sz = 6 + (int)(qSin(m_frame * 0.12 + i) * 3);
            p.setBrush(QColor(255, 228, 55, 195));
            p.setPen(Qt::NoPen);
            p.drawEllipse(sx - sz / 2, sy - sz / 2, sz, sz);
            p.setPen(QPen(QColor(255, 218, 0), 1));
            p.drawLine(sx - sz, sy, sx + sz, sy);
            p.drawLine(sx, sy - sz, sx, sy + sz);
        }
    }
    else if (m_state == TamaState::Tired)
    {
        for (int i = 0; i < 3; i++)
        {
            int yOff = (m_frame * 2 + i * 40) % 80;
            int alpha = qMax(0, 240 - yOff * 3);
            p.setPen(QPen(QColor(148, 168, 228, alpha), 2));
            p.setFont(QFont(AppFonts::Family, 16 + i * 3, QFont::Bold));
            p.drawText((int)(cx + 67 + i * 18), (int)(cy - yOff / 2), "z");
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
    root->setSpacing(20);

    // Canvas
    QFrame *canvasFrame = new QFrame();
    canvasFrame->setStyleSheet("QFrame { background: palette(base); border-radius: 16px; border: 1px solid palette(mid); }");
    QVBoxLayout *cfLayout = new QVBoxLayout(canvasFrame);
    cfLayout->setContentsMargins(0, 0, 0, 0);

    m_canvas = new TamagotchiCanvas();
    m_canvas->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    cfLayout->addWidget(m_canvas);
    root->addWidget(canvasFrame, 3);

    // Right panel
    QFrame *panel = new QFrame();
    panel->setObjectName("tamaPanel");
    panel->setFixedWidth(280);
    panel->setStyleSheet("QFrame#tamaPanel { background: palette(base); border-radius: 16px; border: 1px solid palette(mid); }");

    QVBoxLayout *pv = new QVBoxLayout(panel);
    pv->setContentsMargins(24, 24, 24, 24);
    pv->setSpacing(14);

    QLabel *title = new QLabel("Campus\nTamagotchi");
    title->setStyleSheet(QString("font-size:%1px; font-weight:800; color:palette(text);").arg(AppFonts::Normal + 4));
    title->setAlignment(Qt::AlignCenter);
    title->setWordWrap(true);
    pv->addWidget(title);

    m_lblState = new QLabel("State: Neutral");
    m_lblState->setAlignment(Qt::AlignCenter);
    m_lblState->setStyleSheet(QString("font-size:%1px; font-weight:bold; padding:6px 12px; border-radius:8px;").arg(AppFonts::Small));
    pv->addWidget(m_lblState);

    // Stat bars
    auto makeBar = [&](const QString &label, const QString &color) -> QProgressBar *
    {
        QLabel *lbl = new QLabel(label);
        lbl->setStyleSheet(QString("font-size:%1px; font-weight:600; color:palette(text);").arg(AppFonts::Small));
        pv->addWidget(lbl);
        QProgressBar *bar = new QProgressBar();
        bar->setRange(0, 100);
        bar->setFixedHeight(16);
        bar->setTextVisible(false);
        bar->setStyleSheet(pbStyle(color));
        pv->addWidget(bar);
        return bar;
    };

    m_pbHealth = makeBar("Health 💪", "#e74c3c");
    m_pbMood = makeBar("Mood 😊", "#9b59b6");
    m_pbEnergy = makeBar("Energy ⚡", "#f1c40f");
    m_pbBrain = makeBar("Brain 🧠", "#3498db");

    pv->addSpacing(8);

    m_lblMessage = new QLabel("");
    m_lblMessage->setWordWrap(true);
    m_lblMessage->setAlignment(Qt::AlignCenter);
    m_lblMessage->setStyleSheet(QString(
                                    "font-size:%1px; font-style:italic; font-weight:500; color:palette(text); background:palette(alternate-base);"
                                    "border: 1px solid palette(mid); border-radius:10px; padding:12px;")
                                    .arg(AppFonts::Small));
    m_lblMessage->setMinimumHeight(80);
    pv->addWidget(m_lblMessage);

    pv->addStretch();

    // Buttons
    auto makeBtn = [&](const QString &text, const QString &color) -> QPushButton *
    {
        QPushButton *btn = new QPushButton(text);
        btn->setFixedHeight(44);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setStyleSheet(
            QString("QPushButton { background:%1; color:white; border-radius:10px; font-size:%3px; font-weight:bold; border:none; }"
                    "QPushButton:hover { background:%2; margin-top:-2px; margin-bottom:2px; }")
                .arg(color, QColor(color).darker(115).name())
                .arg(AppFonts::Small));
        return btn;
    };

    m_btnFeed = makeBtn("Feed 🍱", "#27ae60");
    m_btnStudy = makeBtn("Study 📚", "#2980b9");
    m_btnSleep = makeBtn("Sleep 😴", "#8e44ad");

    pv->addWidget(m_btnFeed);
    pv->addWidget(m_btnStudy);
    pv->addWidget(m_btnSleep);

    root->addWidget(panel, 0);

    connect(m_btnFeed, &QPushButton::clicked, this, &UITamagotchi::onFeedClicked);
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
    if (stu)
    {
        s.brain = (int)(stu->getCGPA() / 4.0 * 100.0);
        delete stu;
    }

    // Health: habit streaks + completion
    QVector<Habit *> habits = m_mgr->getHabits(m_userId);
    if (!habits.isEmpty())
    {
        int totalStreak = 0, completed = 0;
        for (auto h : habits)
        {
            totalStreak += h->streak;
            if (h->isCompleted)
                completed++;
        }
        double rate = (double)completed / habits.size();
        double avgS = qMin((double)totalStreak / habits.size() / 7.0, 1.0);
        s.health = (int)(rate * 55 + avgS * 45);
        qDeleteAll(habits);
    }

    // Mood: prayers today
    DailyPrayerStatus prayers = m_mgr->getDailyPrayers(
        m_userId, QDate::currentDate().toString("yyyy-MM-dd"));
    int done = (prayers.getFajr() ? 1 : 0) + (prayers.getDhuhr() ? 1 : 0) +
               (prayers.getAsr() ? 1 : 0) + (prayers.getMaghrib() ? 1 : 0) +
               (prayers.getIsha() ? 1 : 0);
    s.mood = done * 20;

    // Upcoming exam
    QVector<Assessment> all = m_mgr->getStudentAssessments(m_userId);
    QDate today = QDate::currentDate();
    s.daysToExam = 999;
    for (const auto &a : all)
    {
        QDate d = QDate::fromString(a.getDate(), "yyyy-MM-dd");
        if (!d.isValid())
            continue;
        int dl = today.daysTo(d);
        if (dl >= 0 && dl < s.daysToExam)
        {
            s.daysToExam = dl;
            s.examName = a.getTitle() + " (" + a.getCourseName() + ")";
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
    for (const auto &r : att)
    {
        double pct = r.getTotalClasses() > 0
                         ? (double)r.getAttendedClasses() / r.getTotalClasses() * 100.0
                         : 100.0;
        if (pct < 60.0)
        {
            critAtt = true;
            break;
        }
    }
    if (critAtt && s.health < 35)
        return TamaState::Sick;

    if (s.examSoon && s.daysToExam <= 3)
        return TamaState::Stressed;
    if (s.mood < 25 || s.energy < 20)
        return TamaState::Tired;

    int avg = (s.health + s.mood + s.energy + s.brain) / 4;
    if (avg >= 65)
        return TamaState::Happy;
    return TamaState::Neutral;
}

QString UITamagotchi::messageForState(TamaState state, const TamaStats &s)
{
    static const QStringList happy = {"Let's ace this semester! ✨", "GPA going up! 📈", "Prayers done, feeling great!", "Streak maintained! 💪"};
    static const QStringList neutral = {"Time to hit the books...", "What's for dinner? 🍜", "Chapter 5 looks tough.", "Need coffee... ☕"};
    static const QStringList tired = {"Just 5 more minutes... 😴", "Why so many assignments?", "Please log a habit...", "So sleepy..."};
    static const QStringList stressed = {"EXAM SOON!! 😰", "I haven't studied enough!", "Why did I procrastinate?!", "Need to review notes!!"};
    static const QStringList sick = {"Please attend classes... 🤒", "Low attendance is bad...", "Do your habits please!", "I need rest..."};

    const QStringList *pool = &neutral;
    switch (state)
    {
    case TamaState::Happy:
        pool = &happy;
        break;
    case TamaState::Tired:
        pool = &tired;
        break;
    case TamaState::Stressed:
        pool = &stressed;
        break;
    case TamaState::Sick:
        pool = &sick;
        break;
    default:
        break;
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

    static const char *names[] = {"Happy 😊", "Neutral 😐", "Tired 😴", "Stressed 😰", "Sick 🤒"};
    static const char *colors[] = {"#27ae60", "#2980b9", "#7f8c8d", "#e67e22", "#c0392b"};
    int idx = (int)state;
    m_lblState->setText(names[idx]);
    m_lblState->setStyleSheet(
        QString("font-size:%1px; font-weight:bold; color:white; background:%2;"
                "border-radius:8px; padding:6px 12px;")
            .arg(AppFonts::Small)
            .arg(colors[idx]));

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
    m_stats.mood = qMin(100, m_stats.mood + 15);
    m_stats.health = qMin(100, m_stats.health + 10);
    applyStats();
}

void UITamagotchi::onStudyClicked()
{
    m_energy = qMin(100, m_energy + 20);
    m_stats.energy = m_energy;
    m_stats.brain = qMin(100, m_stats.brain + 5);
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
    m_stats = computeStats();
    applyStats();
}

// ─── helpers ─────────────────────────────────────────────────────────────────
QString UITamagotchi::pbStyle(const QString &color)
{
    return QString(
               "QProgressBar { background:palette(mid); border-radius:8px; border: none; }"
               "QProgressBar::chunk { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 %1, stop:1 %2); border-radius:8px; }")
        .arg(QColor(color).darker(110).name(), color);
}

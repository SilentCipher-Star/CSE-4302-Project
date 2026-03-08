#pragma once

#include <QString>
#include <QColor>
#include <QDate>
#include <memory>

/**
 * Decorator Pattern – Notice System
 *
 * Demonstrates composition over inheritance: each decorator wraps an INotice
 * and adds visual / behavioural enhancements without touching the stored data.
 *
 *   Component      : INotice
 *   Concrete comp  : ConcreteNotice
 *   Decorator base : NoticeDecorator
 *   Decorators     : UrgentNotice, PinnedNotice, ExpiringNotice
 *
 * Usage:
 *   auto n = buildDecoratedNotice(subject, body, author, date, isUrgent, isPinned, expiresOn);
 *   QString badges     = n->getBadges();          // e.g. "[URGENT] [PINNED]"
 *   QColor  highlight  = n->getHighlightColor();  // e.g. red tint
 *   bool    priority   = n->isPriority();         // sort before ordinary notices
 */

// ---------------------------------------------------------------------------
// Component interface
// ---------------------------------------------------------------------------
class INotice
{
public:
    virtual QString getSubject()        const = 0;
    virtual QString getBody()           const = 0;
    virtual QString getAuthor()         const = 0;
    virtual QString getDate()           const = 0;
    /// Combined badge string, e.g. "[URGENT] [PINNED]"
    virtual QString getBadges()         const = 0;
    /// Background highlight colour; invalid QColor means no tint
    virtual QColor  getHighlightColor() const = 0;
    /// Returns true when this notice should be sorted before ordinary notices
    virtual bool    isPriority()        const = 0;
    virtual ~INotice() = default;
};

// ---------------------------------------------------------------------------
// Concrete component – plain notice with no extra decoration
// ---------------------------------------------------------------------------
class ConcreteNotice : public INotice
{
public:
    ConcreteNotice(QString subject, QString body, QString author, QString date)
        : m_subject(std::move(subject)), m_body(std::move(body)),
          m_author(std::move(author)),   m_date(std::move(date)) {}

    QString getSubject()        const override { return m_subject; }
    QString getBody()           const override { return m_body; }
    QString getAuthor()         const override { return m_author; }
    QString getDate()           const override { return m_date; }
    QString getBadges()         const override { return {}; }
    QColor  getHighlightColor() const override { return {}; }   // invalid = no tint
    bool    isPriority()        const override { return false; }

private:
    QString m_subject, m_body, m_author, m_date;
};

// ---------------------------------------------------------------------------
// Abstract decorator – forwards every call to the wrapped INotice
// ---------------------------------------------------------------------------
class NoticeDecorator : public INotice
{
public:
    explicit NoticeDecorator(std::unique_ptr<INotice> wrapped)
        : m_wrapped(std::move(wrapped)) {}

    QString getSubject()        const override { return m_wrapped->getSubject(); }
    QString getBody()           const override { return m_wrapped->getBody(); }
    QString getAuthor()         const override { return m_wrapped->getAuthor(); }
    QString getDate()           const override { return m_wrapped->getDate(); }
    QString getBadges()         const override { return m_wrapped->getBadges(); }
    QColor  getHighlightColor() const override { return m_wrapped->getHighlightColor(); }
    bool    isPriority()        const override { return m_wrapped->isPriority(); }

protected:
    std::unique_ptr<INotice> m_wrapped;
};

// ---------------------------------------------------------------------------
// UrgentNotice – red background tint, sorted first, "[URGENT]" badge
// ---------------------------------------------------------------------------
class UrgentNotice : public NoticeDecorator
{
public:
    explicit UrgentNotice(std::unique_ptr<INotice> wrapped)
        : NoticeDecorator(std::move(wrapped)) {}

    QString getBadges() const override
    {
        const QString inner = m_wrapped->getBadges();
        return inner.isEmpty() ? QString("[URGENT]")
                               : inner + " [URGENT]";
    }

    // Red tint always takes visual priority
    QColor getHighlightColor() const override { return QColor(255, 210, 210); }

    bool isPriority() const override { return true; }
};

// ---------------------------------------------------------------------------
// PinnedNotice – yellow tint (unless red already set), sorted first
// ---------------------------------------------------------------------------
class PinnedNotice : public NoticeDecorator
{
public:
    explicit PinnedNotice(std::unique_ptr<INotice> wrapped)
        : NoticeDecorator(std::move(wrapped)) {}

    QString getBadges() const override
    {
        const QString inner = m_wrapped->getBadges();
        return inner.isEmpty() ? QString("[PINNED]")
                               : inner + " [PINNED]";
    }

    QColor getHighlightColor() const override
    {
        const QColor inner = m_wrapped->getHighlightColor();
        return inner.isValid() ? inner : QColor(255, 255, 200);   // yellow tint
    }

    bool isPriority() const override { return true; }
};

// ---------------------------------------------------------------------------
// ExpiringNotice – orange tint when near/past expiry, expiry date in badge
// ---------------------------------------------------------------------------
class ExpiringNotice : public NoticeDecorator
{
public:
    ExpiringNotice(std::unique_ptr<INotice> wrapped, QString expiresOn)
        : NoticeDecorator(std::move(wrapped)), m_expiresOn(std::move(expiresOn)) {}

    QString getBadges() const override
    {
        const QDate expiry = QDate::fromString(m_expiresOn, Qt::ISODate);
        const QDate today  = QDate::currentDate();
        QString badge;
        if (expiry.isValid())
        {
            if (expiry < today)
                badge = "[EXPIRED]";
            else if (expiry <= today.addDays(3))
                badge = "[EXPIRES " + m_expiresOn + "]";
            else
                badge = "[EXP: " + m_expiresOn + "]";
        }
        const QString inner = m_wrapped->getBadges();
        if (inner.isEmpty())  return badge;
        return badge.isEmpty() ? inner : inner + " " + badge;
    }

    QColor getHighlightColor() const override
    {
        const QColor inner = m_wrapped->getHighlightColor();
        if (inner.isValid())
            return inner;   // stronger (red/yellow) decorator wins

        const QDate expiry = QDate::fromString(m_expiresOn, Qt::ISODate);
        if (expiry.isValid() && expiry <= QDate::currentDate().addDays(3))
            return QColor(255, 235, 200);   // orange tint = expiring soon / already expired
        return {};
    }

private:
    QString m_expiresOn;
};

// ---------------------------------------------------------------------------
// Factory helper – wraps a ConcreteNotice with the correct decorator chain
// ---------------------------------------------------------------------------
inline std::unique_ptr<INotice> buildDecoratedNotice(
    const QString &subject,
    const QString &body,
    const QString &author,
    const QString &date,
    bool isUrgent,
    bool isPinned,
    const QString &expiresOn)
{
    std::unique_ptr<INotice> notice =
        std::make_unique<ConcreteNotice>(subject, body, author, date);

    // Apply decorators: ExpiringNotice first (innermost), then Pinned, then Urgent
    if (!expiresOn.isEmpty())
        notice = std::make_unique<ExpiringNotice>(std::move(notice), expiresOn);
    if (isPinned)
        notice = std::make_unique<PinnedNotice>(std::move(notice));
    if (isUrgent)
        notice = std::make_unique<UrgentNotice>(std::move(notice));

    return notice;
}

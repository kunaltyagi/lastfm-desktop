#ifndef ACTIVITY_LIST_MODEL_H_
#define ACTIVITY_LIST_MODEL_H_

#include <lastfm/ws.h>
#include <lastfm/Track>
#include <QImage>
#include <QPixmap>
#include <QIcon>

namespace unicorn { class Session; }

class ImageTrack : public QObject, public lastfm::Track {
Q_OBJECT
public:
    ImageTrack(const lastfm::Track& t):QObject(), lastfm::Track(t) {}
    ImageTrack(ImageTrack& t ):QObject(), lastfm::Track(t){}
    void fetchImage() {
        QNetworkReply* r = lastfm::nam()->get(QNetworkRequest(imageUrl(lastfm::Small, false )));
        connect( r, SIGNAL(finished()), SLOT(onGotImage()));
    }

    ImageTrack& operator=(const ImageTrack& t){ *this = ImageTrack(t); return *this; }
    const QImage& image() const { return m_image; }

signals:
    void imageUpdated();

private:
    QImage m_image;

private slots:
    void onGotImage() {
        qDebug() << "Finished fetching image for track: " << toString();
        sender()->deleteLater();
        QNetworkReply* reply = (QNetworkReply*) sender();
        if (reply->error() == QNetworkReply::NoError) {
            QPixmap p;
            p.loadFromData(((QNetworkReply*)sender())->readAll());
            if (!p.isNull()) {
                m_image = p.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation).toImage();
            }
        }
        emit imageUpdated();
    }
};



#include <QAbstractItemModel>
class ActivityListModel : public QAbstractItemModel {
    Q_OBJECT
public:
    ActivityListModel() { 
        loveIcon.addFile( ":/activity_min_love_OFF_REST.png", QSize(), QIcon::Normal, QIcon::Off );
        loveIcon.addFile( ":/activity_min_love_ON_REST.png", QSize(), QIcon::Normal, QIcon::On );
        loveIcon.addFile( ":/activity_min_love_OFF_HOVER.png", QSize(), QIcon::Selected, QIcon::Off );
        loveIcon.addFile( ":/activity_min_love_ON_HOVER.png", QSize(), QIcon::Selected, QIcon::On );

        tagIcon.addFile( ":/activity_min_tag_REST.png", QSize(), QIcon::Normal, QIcon::Off );
        tagIcon.addFile( ":/activity_min_tag_HOVER.png", QSize(), QIcon::Selected, QIcon::Off );
    }

    enum {
        TrackNameRole = Qt::UserRole,
        ArtistNameRole,
        LovedRole,
        TimeStampRole,
        HoverStateRole,
        TrackRole
    };

    virtual QModelIndex index( int row, int column, const QModelIndex& parent = QModelIndex()) const { return createIndex( row, column ); }
    virtual QModelIndex parent( const QModelIndex& index ) const { return QModelIndex(); }
    virtual int rowCount( const QModelIndex& parent = QModelIndex()) const { return parent.isValid() ? 0 : m_tracks.length(); }
    virtual int columnCount( const QModelIndex& parent = QModelIndex()) const { return parent.isValid() ? 0 : 3; }
    virtual QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;
    virtual bool setData( const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
    virtual Qt::ItemFlags flags( const QModelIndex& index ) const;
    virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const {
        if( orientation != Qt::Horizontal ||
            role != Qt::DisplayRole )
            return QVariant();
        
        switch( section ) {
            case 0: return tr( "Item" );
            case 1: return tr( "Loved" );
        }

        return QVariant();
    }

public slots:
    void onFoundIPodScrobbles( const QList<lastfm::Track>& tracks );
    void onTrackStarted( const Track& );
    void onSessionChanged( unicorn::Session* session );
    void onTrackLoveToggled();

private:
    void read();
    QList<ImageTrack> m_tracks;
    QString m_path;
    QIcon loveIcon;
    QIcon tagIcon;
    QModelIndex hoverIndex;


private slots:
    void write() const;

};

#endif //ACTIVITY_LIST_MODEL_H_

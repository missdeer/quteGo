/*
 * board.h
 */

#ifndef ANALYZEDLG_H
#define ANALYZEDLG_H

#include <forward_list>
#include <map>
#include <vector>

#include "defines.h"
#include "goboard.h"
#include "gogame.h"
#include "qgtp.h"
#include "setting.h"
#include "ui_analyze_gui.h"

class MainWindow;

class AnalyzeDialog
    : public QMainWindow
    , public Ui::AnalyzeDialog
    , public GTP_Eval_Controller
{
    Q_OBJECT

    QList<Engine> m_engines;

    struct job;
    friend struct job;
    struct display
    {
        std::vector<job *> jobs;
        QMap<int, job *>   map;
        QStandardItemModel model;
    };
    /* Keeps all jobs so that they can be deleted once the dialog is destroyed.
       Or we could clear it manually if we have no running jobs in the displays.
     */
    std::forward_list<job> m_all_jobs;
    struct display         m_jobs, m_done;

    int m_job_count = 0;

    QString m_current_komi;
    enum class engine_komi
    {
        dflt,
        maybe_swap,
        do_swap,
        both
    };

    struct job
    {
        AnalyzeDialog *m_dlg;
        QString        m_title;
        go_game_ptr    m_game;
        MainWindow    *m_win {};
        /* We connect to the window's close event, and keep this connection
           around so we can delete it on close.  */
        QMetaObject::Connection m_connection;
        int                     m_n_seconds;
        int                     m_n_lines;
        engine_komi             m_komi_type;
        bool                    m_comments;

        std::vector<game_state *> m_queue;
        std::vector<game_state *> m_queue_flipped;
        size_t                    m_initial_size;
        size_t                    m_done = 0;

        display *m_display;
        int      m_idx;

        job(AnalyzeDialog *dlg, QString &title, go_game_ptr gr, int n_seconds, int n_lines, engine_komi, bool comments);
        ~job();
        game_state *select_request(bool pop);
        void        show_window(bool done);
    };

    job          *m_requester;
    int           m_seconds_count;
    QIntValidator m_seconds_vald {1, 86400};
    QIntValidator m_lines_vald {1, 100};

    QString m_last_dir;

    void queue_next();

    void select_file();
    void start_engine();
    void start_job();

    /* Maintaining the job queue listviews and assorted data structures.  */
    void insert_job(display &, QListView *, job *);
    void remove_job(display &, job *);

    void update_progress();
    void update_buttons(display &, QListView *, QProgressBar *, QToolButton *, QToolButton *);
    job *selected_job(bool done);
    void open_in_progress_window(bool done);
    void discard_job(bool done);

    /* Virtuals from Gtp_Controller.  */
    virtual void eval_received(const QString &, int, bool) override;
    virtual void analyzer_state_changed() override;
    virtual void notice_analyzer_id(const analyzer_id &, bool) override;

    virtual void closeEvent(QCloseEvent *) override;

public:
    AnalyzeDialog(QWidget *parent, const QString &filename);
    ~AnalyzeDialog();

    /* Used internally, and also called when the settings change.  */
    void update_engines();

    /* Virtuals from Gtp_Controller.  */
    virtual void gtp_startup_success(GTP_Process *) override;
    virtual void gtp_exited(GTP_Process *) override;
    virtual void gtp_failure(GTP_Process *, const QString &) override;
};

extern AnalyzeDialog *analyze_dialog;

#endif

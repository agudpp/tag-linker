#include "tag_handler_widget.h"

#include <core/utils/string_utils.h>
#include <core/debug/Debug.h>

#include <QDebug>
#include <QEvent>
#include <QKeyEvent>

#include <ui_client/utils/converter.h>

#include "ui_tag_handler_widget.h"


void
TagHandlerWidget::removeCurrentSelTag(void)
{
  if (!selected_tags_->hasSelection()) {
    return;
  }
  TagWidget* current = selected_tags_->selected();
  selected_tags_->popTag(current);
  emit tagRemoved(current->tag());
  current->freeObject();
}

void
TagHandlerWidget::selectNextTag(TagListHandler* tag_handler, bool left_dir, bool only_if_selected, bool emit_signal)
{
  if (!tag_handler->hasSelection() && only_if_selected) {
    return;
  }

  if (left_dir) {
    emit_signal = tag_handler->selectPrev() && emit_signal;
  } else {
    emit_signal = tag_handler->selectNext() && emit_signal;
  }
  if (emit_signal && tag_handler->hasSelection()) {
    emit tagSelected(tag_handler->selected()->tag());
  }
}


bool
TagHandlerWidget::lineEditEventFilter(QEvent *event)
{
  const bool is_key_release = event->type() == QEvent::KeyRelease;
  const bool is_key_press = event->type() == QEvent::KeyPress;

  if(!is_key_press && !is_key_release) {
    return false;
  }

  QKeyEvent *ke = static_cast<QKeyEvent *>(event);
  // We avoid propagaiting the tab key if pressed in here.
  if (is_key_press) {
    if (ke->key() == Qt::Key_Tab) {
      event->accept();
      return true;
    }
    return false;
  }

  const bool shift_pressed = ke->modifiers() & Qt::ShiftModifier;

  if (ke->key() == Qt::Key_Backspace) {
    // Check if we need to select / remove tag when empty text
    if (ui->lineEdit->text().isEmpty()) {
      if (selected_tags_->hasSelection()) {
        removeCurrentSelTag();
      } else if (selected_tags_->hasTags()){
        selected_tags_->select(selected_tags_->last());
        emit tagSelected(selected_tags_->selected()->tag());
      }
    }
  } else if (ke->key() == Qt::Key_Tab) {
    if (selected_tags_->hasSelection() || suggested_tags_->hasSelection()) {
      selectNextTag(selected_tags_, shift_pressed, true);
      selectNextTag(suggested_tags_, shift_pressed, true, false);
    } else {
      // autocomplete?
      const std::string current_text = ui->lineEdit->text().toStdString();
      const std::string expanded = core::StringUtils::shortestCommonSuffix(suggestedTagsTexts(), current_text);
      qDebug() << "current_text: " << current_text.c_str() << " expanded: " << expanded.c_str();
      if (expanded == current_text) {
        // we need to jump to the suggested tags to select one
        selectNextTag(suggested_tags_, shift_pressed, false, false);
      } else {
        ui->lineEdit->setText(expanded.c_str());
      }
    }
    event->accept();
    qDebug() << "accepting event tab";
    ui->lineEdit->setFocus();
    return true;
  } else if (ke->key() == Qt::Key_Escape) {
    selected_tags_->unselect(selected_tags_->selected());
    suggested_tags_->unselect(suggested_tags_->selected());
    emit escapePressed();
  } else if (ke->key() == Qt::Key_Return) {
    if (suggested_tags_->hasSelection()) {
      TagWidget* sel_tag = suggested_tags_->selected();
      suggested_tags_->popTag(sel_tag);
      selected_tags_->addTag(sel_tag);
      emit tagSelected(sel_tag->tag());
    }
  }

  return false;
}

void
TagHandlerWidget::lineEditTextChanged(const QString& text)
{
  selected_tags_->unselect(selected_tags_->selected());
  emit inputTextChanged(text);
}



TagHandlerWidget::TagHandlerWidget(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::TagHandlerWidget)
{
  ui->setupUi(this);
  selected_tags_ = new TagListHandler;
  suggested_tags_ = new TagListHandler;
  selected_tags_->setFocusPolicy(Qt::FocusPolicy::NoFocus);
  suggested_tags_->setFocusPolicy(Qt::FocusPolicy::NoFocus);

  ui->verticalLayout->addWidget(selected_tags_);
  ui->verticalLayout->addWidget(suggested_tags_);

  installEventFilter(this);
  ui->lineEdit->installEventFilter(this);

  QObject::connect(ui->lineEdit, &QLineEdit::textChanged, this, &TagHandlerWidget::lineEditTextChanged);
}


TagHandlerWidget::~TagHandlerWidget()
{
  delete ui;
}


void
TagHandlerWidget::setSuggestedTags(const std::set<Tag::ConstPtr>& tags)
{

  suggested_tags_->setTags(Converter::toWidget(tags));
}


bool
TagHandlerWidget::eventFilter(QObject *object, QEvent *event)
{
  if (object == ui->lineEdit) {
    return lineEditEventFilter(event);
  }

  return false;
}

std::vector<std::string>
TagHandlerWidget::suggestedTagsTexts(void) const
{
  std::vector<std::string> tags_text;
  const std::vector<TagWidget*>& tags = suggested_tags_->tags();
  for (const TagWidget* tag : tags) {
    ASSERT_PTR(tag);
    ASSERT_PTR(tag->tag().get());
    tags_text.push_back(tag->tag()->text());
  }
  return tags_text;
}

std::vector<std::string>
TagHandlerWidget::selectedTagsTexts(void) const
{
  std::vector<std::string> tags_text;
  const std::vector<TagWidget*>& tags = selected_tags_->tags();
  for (const TagWidget* tag : tags) {
    tags_text.push_back(tag->tag()->text());
  }
  return tags_text;
}

QString
TagHandlerWidget::currentText(void) const
{
  return ui->lineEdit->text();
}
/*
   DScope Application

   Copyright (C) 2008,2009:
         Daniel Roggen, droggen@gmail.com

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "helpwindow.h"
#include "ui_helpwindow.h"

HelpWindow::HelpWindow(QString message,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::HelpWindow)
{
   ui->setupUi(this);

   mainFrame = new QFrame(this);

   label = new QLabel(this);
   label->setWordWrap(true);
   label->setOpenExternalLinks(true);
   label->setFixedWidth(550);

   ui->scrollArea->setWidget(label);

   label->setText(message);
}

HelpWindow::~HelpWindow()
{
    delete ui;
}

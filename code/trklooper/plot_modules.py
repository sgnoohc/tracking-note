#!/bin/env python

import ROOT as r
import sys

tf = r.TFile("/hadoop/cms/store/user/slava77/CMSSW_10_4_0_patch1-tkNtuple/pass-e072c1a/27411.0_TenMuExtendedE_0_200/trackingNtuple.root")
ttree = tf.Get("trackingNtuple/tree")

c1 = r.TCanvas()

def plot(cuts, color, same):
    ttree.Draw("ph2_y:ph2_x", cuts, same)
    ttree.Draw("sqrt(ph2_x**2+ph2_y**2):ph2_z", cuts, same)


def rz_all_side():
    # rz all with color codes on ph2_side
    ttree.SetMarkerColor(1)
    ttree.Draw("sqrt(ph2_x**2+ph2_y**2):ph2_z", "")
    ttree.SetMarkerColor(4)
    ttree.Draw("sqrt(ph2_x**2+ph2_y**2):ph2_z", "ph2_side==1", "same")
    ttree.SetMarkerColor(2)
    ttree.Draw("sqrt(ph2_x**2+ph2_y**2):ph2_z", "ph2_side==2", "same")
    ttree.SetMarkerColor(1)
    ttree.Draw("sqrt(ph2_x**2+ph2_y**2):ph2_z", "ph2_side==3", "same")
    c1.SaveAs("plots/lin/rz_side.pdf")
    c1.SaveAs("plots/lin/rz_side.png")

def rz_side3_layer1_order0():
    # rz layer 1 barrel
    ttree.SetMarkerColor(1)
    ttree.Draw("sqrt(ph2_x**2+ph2_y**2):ph2_z", "ph2_side==3&&ph2_order==0&&ph2_layer==1")
    c1.SaveAs("plots/lin/rz_side3_layer1_order0.pdf")
    c1.SaveAs("plots/lin/rz_side3_layer1_order0.png")

def xy_side3_layer1_order0():
    # xy layer 1 barrel
    ttree.SetMarkerColor(1)
    ttree.Draw("ph2_y:ph2_x", "ph2_side==3&&ph2_order==0&&ph2_layer==1")
    c1.SaveAs("plots/lin/xy_side3_layer1_order0.pdf")
    c1.SaveAs("plots/lin/xy_side3_layer1_order0.png")

def rz_side1_layer1_order0():
    # rz layer 1 barrel
    ttree.SetMarkerColor(1)
    ttree.Draw("sqrt(ph2_x**2+ph2_y**2):ph2_z", "ph2_side==1&&ph2_order==0&&ph2_layer==1")
    c1.SaveAs("plots/lin/rz_side1_layer1_order0.pdf")
    c1.SaveAs("plots/lin/rz_side1_layer1_order0.png")

def rz_side1_layer1_order1():
    # rz layer 1 barrel
    ttree.SetMarkerColor(1)
    ttree.Draw("sqrt(ph2_x**2+ph2_y**2):ph2_z", "ph2_side==1&&ph2_order==1&&ph2_layer==1")
    c1.SaveAs("plots/lin/rz_side1_layer1_order1.pdf")
    c1.SaveAs("plots/lin/rz_side1_layer1_order1.png")

def rz_side3_order0():
    # rz layer 1 barrel
    ttree.SetMarkerColor(1)
    ttree.Draw("sqrt(ph2_x**2+ph2_y**2):ph2_z", "ph2_side==3&&ph2_order==0")
    c1.SaveAs("plots/lin/rz_side3_order0.pdf")
    c1.SaveAs("plots/lin/rz_side3_order0.png")

def xy_side3_order0():
    # xy layer 1 barrel
    ttree.SetMarkerColor(1)
    ttree.Draw("ph2_y:ph2_x", "ph2_side==3&&ph2_order==0")
    c1.SaveAs("plots/lin/xy_side3_order0.pdf")
    c1.SaveAs("plots/lin/xy_side3_order0.png")

def xy_side3_order0_rodcolors():
    colors = [1, 2]
    ttree.Draw("ph2_y:ph2_x", "ph2_side==3&&ph2_order==0")
    # xy layer 1 barrel
    for i in xrange(1, 77):
        ttree.SetMarkerColor(colors[i%2])
        ttree.Draw("ph2_y:ph2_x", "ph2_side==3&&ph2_order==0&&ph2_rod=={}".format(i), "same")
    c1.SaveAs("plots/lin/xy_side3_order0_rodcolor.pdf")
    c1.SaveAs("plots/lin/xy_side3_order0_rodcolor.png")

def rz_side3_order0_rodcolors():
    colors = [1, 2]
    ttree.Draw("sqrt(ph2_x**2+ph2_y**2):ph2_z", "ph2_side==3&&ph2_order==0")
    # rz layer 1 barrel
    for i in xrange(1, 79):
        ttree.SetMarkerColor(colors[i%2])
        ttree.Draw("sqrt(ph2_x**2+ph2_y**2):ph2_z", "ph2_side==3&&ph2_order==0&&ph2_rod=={}".format(i), "same")
    c1.SaveAs("plots/lin/rz_side3_order0_rodcolor.pdf")
    c1.SaveAs("plots/lin/rz_side3_order0_rodcolor.png")

def rz_side3_order0_rodcolors_modcolors():
    colors = [1, 2, 4, 8]
    ttree.Draw("sqrt(ph2_x**2+ph2_y**2):ph2_z", "ph2_side==3&&ph2_order==0")

    cuts = [
            "ph2_rod%2==1&&ph2_module%2==1",
            "ph2_rod%2==0&&ph2_module%2==1",
            "ph2_rod%2==1&&ph2_module%2==0",
            "ph2_rod%2==0&&ph2_module%2==0",
            ]
    # rz layer 1 barrel
    for i, cut in enumerate(cuts):
        ttree.SetMarkerColor(colors[i])
        ttree.Draw("sqrt(ph2_x**2+ph2_y**2):ph2_z", "ph2_side==3&&ph2_order==0&&{}".format(cut), "same")
    c1.SaveAs("plots/lin/rz_side3_order0_rodcolor_modcolor.pdf")
    c1.SaveAs("plots/lin/rz_side3_order0_rodcolor_modcolor.png")

def xy_side3_order0_rodall():
    for i in xrange(1, 79):
        ttree.SetMarkerColor(1)
        ttree.Draw("ph2_y:ph2_x", "ph2_side==3&&ph2_order==0")
        ttree.SetMarkerColor(2)
        ttree.Draw("ph2_y:ph2_x", "ph2_side==3&&ph2_order==0&&ph2_rod=={}".format(i), "same")
        c1.SaveAs("plots/lin/xy_side3_order0_rod{}.pdf".format(i))
        c1.SaveAs("plots/lin/xy_side3_order0_rod{}.png".format(i))

def rz_subdet4():
    # rz layer 1 barrel
    ttree.Draw("sqrt(ph2_x**2+ph2_y**2):ph2_z", "")
    ttree.SetMarkerColor(1)
    ttree.Draw("sqrt(ph2_x**2+ph2_y**2):ph2_z", "ph2_subdet==4")
    c1.SaveAs("plots/lin/rz_subdet4.pdf")
    c1.SaveAs("plots/lin/rz_subdet4.png")

def xy_side1_layer1_ringall():
    for i in xrange(1, 16):
        ttree.SetMarkerColor(1)
        ttree.Draw("ph2_y:ph2_x", "ph2_side==1&&ph2_layer==1")
        ttree.SetMarkerColor(2)
        ttree.Draw("ph2_y:ph2_x", "ph2_side==1&&ph2_layer==1&&ph2_ring=={}".format(i))
        c1.SaveAs("plots/lin/xy_side1_layer1_ring{}.pdf".format(i))
        c1.SaveAs("plots/lin/xy_side1_layer1_ring{}.png".format(i))

def xy_side1_layer1_ring2_moduleall():
    for i in xrange(1, 16):
        ttree.SetMarkerColor(1)
        ttree.Draw("ph2_y:ph2_x", "ph2_side==1&&ph2_layer==1&&ph2_ring==2")
        ttree.SetMarkerColor(2)
        ttree.Draw("ph2_y:ph2_x", "ph2_side==1&&ph2_layer==1&&ph2_ring==2&&ph2_module=={}".format(i), "same")
        c1.SaveAs("plots/lin/xy_side1_layer1_ring2_module{}.pdf".format(i))
        c1.SaveAs("plots/lin/xy_side1_layer1_ring2_module{}.png".format(i))

def xy_side1_layer1_moduleall():
    for i in xrange(1, 16):
        ttree.SetMarkerColor(1)
        ttree.Draw("ph2_y:ph2_x", "ph2_side==1&&ph2_layer==1")
        ttree.SetMarkerColor(2)
        ttree.Draw("ph2_y:ph2_x", "ph2_side==1&&ph2_layer==1&&ph2_module=={}".format(i), "same")
        c1.SaveAs("plots/lin/xy_side1_layer1_module{}.pdf".format(i))
        c1.SaveAs("plots/lin/xy_side1_layer1_module{}.png".format(i))

def rz_side1_layer1_moduleall():
    for i in xrange(1, 16):
        ttree.SetMarkerColor(1)
        ttree.Draw("sqrt(ph2_x**2+ph2_y**2):ph2_z", "ph2_side==1&&ph2_layer==1")
        ttree.SetMarkerColor(2)
        ttree.Draw("sqrt(ph2_x**2+ph2_y**2):ph2_z", "ph2_side==1&&ph2_layer==1&&ph2_module=={}".format(i), "same")
        c1.SaveAs("plots/lin/rz_side1_layer1_module{}.pdf".format(i))
        c1.SaveAs("plots/lin/rz_side1_layer1_module{}.png".format(i))

def rz_side1_layer1_order1_moduleall():
    for i in xrange(1, 16):
        ttree.SetMarkerColor(1)
        ttree.Draw("sqrt(ph2_x**2+ph2_y**2):ph2_z", "ph2_side==1&&ph2_layer==1&&ph2_order==1")
        ttree.SetMarkerColor(2)
        ttree.Draw("sqrt(ph2_x**2+ph2_y**2):ph2_z", "ph2_side==1&&ph2_layer==1&&ph2_order==1&&ph2_module=={}".format(i), "same")
        c1.SaveAs("plots/lin/rz_side1_layer1_order1_module{}.pdf".format(i))
        c1.SaveAs("plots/lin/rz_side1_layer1_order1_module{}.png".format(i))

def xy_side1_layer1_order1_moduleall():
    for i in xrange(1, 16):
        ttree.SetMarkerColor(1)
        ttree.Draw("ph2_y:ph2_x", "ph2_side==1&&ph2_layer==1&&ph2_order==1")
        ttree.SetMarkerColor(2)
        ttree.Draw("ph2_y:ph2_x", "ph2_side==1&&ph2_layer==1&&ph2_order==1&&ph2_module=={}".format(i), "same")
        c1.SaveAs("plots/lin/xy_side1_layer1_order1_module{}.pdf".format(i))
        c1.SaveAs("plots/lin/xy_side1_layer1_order1_module{}.png".format(i))

def rz_side1_layer1_order0_moduleall():
    for i in xrange(1, 16):
        ttree.SetMarkerColor(1)
        ttree.Draw("sqrt(ph2_x**2+ph2_y**2):ph2_z", "ph2_side==1&&ph2_layer==1&&ph2_order==0")
        ttree.SetMarkerColor(2)
        ttree.Draw("sqrt(ph2_x**2+ph2_y**2):ph2_z", "ph2_side==1&&ph2_layer==1&&ph2_order==0&&ph2_module=={}".format(i), "same")
        c1.SaveAs("plots/lin/rz_side1_layer1_order0_module{}.pdf".format(i))
        c1.SaveAs("plots/lin/rz_side1_layer1_order0_module{}.png".format(i))

def xy_side1_layer1_order0_moduleall():
    for i in xrange(1, 16):
        ttree.SetMarkerColor(1)
        ttree.Draw("ph2_y:ph2_x", "ph2_side==1&&ph2_layer==1&&ph2_order==0")
        ttree.SetMarkerColor(2)
        ttree.Draw("ph2_y:ph2_x", "ph2_side==1&&ph2_layer==1&&ph2_order==0&&ph2_module=={}".format(i), "same")
        c1.SaveAs("plots/lin/xy_side1_layer1_order0_module{}.pdf".format(i))
        c1.SaveAs("plots/lin/xy_side1_layer1_order0_module{}.png".format(i))

def xy_side1_order0_moduleall():
    for i in xrange(1, 37):
        ttree.SetMarkerColor(1)
        ttree.Draw("ph2_y:ph2_x", "ph2_side==1&&ph2_order==0")
        ttree.SetMarkerColor(2)
        ttree.Draw("ph2_y:ph2_x", "ph2_side==1&&ph2_order==0&&ph2_module=={}".format(i), "same")
        c1.SaveAs("plots/lin/xy_side1_layer1_order0_module{}.pdf".format(i))
        c1.SaveAs("plots/lin/xy_side1_layer1_order0_module{}.png".format(i))

def rz_isLowerUpper():
    # rz layer 1 barrel
    ttree.SetMarkerColor(1)
    ttree.Draw("sqrt(ph2_x**2+ph2_y**2):ph2_z", "")
    ttree.SetMarkerColor(2)
    ttree.Draw("sqrt(ph2_x**2+ph2_y**2):ph2_z", "ph2_isLower==1", "same")
    c1.SaveAs("plots/lin/rz_isLowerUpper.pdf")
    c1.SaveAs("plots/lin/rz_isLowerUpper.png")

def rz_layerleq3_side3_isLowerUpper():
    # rz layer 1 barrel
    ttree.SetMarkerColor(1)
    ttree.Draw("sqrt(ph2_x**2+ph2_y**2):ph2_z", "ph2_layer<=3&&ph2_side==3")
    ttree.SetMarkerColor(2)
    ttree.Draw("sqrt(ph2_x**2+ph2_y**2):ph2_z", "ph2_layer<=3&&ph2_side==3&&ph2_isLower==1", "same")
    c1.SaveAs("plots/lin/rz_layerleq3_side3_isLowerUpper.pdf")
    c1.SaveAs("plots/lin/rz_layerleq3_side3_isLowerUpper.png")

def rz_layerleq3_side1_isLowerUpper():
    # rz layer 1 barrel
    ttree.SetMarkerColor(1)
    ttree.Draw("sqrt(ph2_x**2+ph2_y**2):ph2_z", "ph2_layer<=3&&ph2_side==1")
    ttree.SetMarkerColor(2)
    ttree.Draw("sqrt(ph2_x**2+ph2_y**2):ph2_z", "ph2_layer<=3&&ph2_side==1&&ph2_isLower==1", "same")
    c1.SaveAs("plots/lin/rz_layerleq3_side1_isLowerUpper.pdf")
    c1.SaveAs("plots/lin/rz_layerleq3_side1_isLowerUpper.png")

def rz_layergeq4_side3_isLowerUpper():
    # rz layer 1 barrel
    ttree.SetMarkerColor(1)
    ttree.Draw("sqrt(ph2_x**2+ph2_y**2):ph2_z", "ph2_layer>=4&&ph2_side==3")
    ttree.SetMarkerColor(2)
    ttree.Draw("sqrt(ph2_x**2+ph2_y**2):ph2_z", "ph2_layer>=4&&ph2_side==3&&ph2_isLower==1", "same")
    c1.SaveAs("plots/lin/rz_layergeq4_side3_isLowerUpper.pdf")
    c1.SaveAs("plots/lin/rz_layergeq4_side3_isLowerUpper.png")

def rz_layergeq4_side1_isLowerUpper():
    # rz layer 1 barrel
    ttree.SetMarkerColor(1)
    ttree.Draw("sqrt(ph2_x**2+ph2_y**2):ph2_z", "ph2_layer>=4&&ph2_side==1")
    ttree.SetMarkerColor(2)
    ttree.Draw("sqrt(ph2_x**2+ph2_y**2):ph2_z", "ph2_layer>=4&&ph2_side==1&&ph2_isLower==1", "same")
    c1.SaveAs("plots/lin/rz_layergeq4_side1_isLowerUpper.pdf")
    c1.SaveAs("plots/lin/rz_layergeq4_side1_isLowerUpper.png")

if __name__ == "__main__":

    # rz_side1_layer1_order0_moduleall()
    # xy_side1_layer1_order0_moduleall()
    # xy_side1_order0_moduleall()
    # rz_isLowerUpper()
    # rz_side3_isLowerUpper()
    # rz_layerleq3_side3_isLowerUpper()
    rz_layergeq4_side1_isLowerUpper()
    rz_layergeq4_side3_isLowerUpper()

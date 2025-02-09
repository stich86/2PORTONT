<?xml version='1.0'?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns:doc="http://nwalsh.com/xsl/documentation/1.0"
                exclude-result-prefixes="doc"
                version='1.0'>

<!-- ********************************************************************
     $Id: subtitles.xsl,v 1.1.1.1 2009/05/07 11:14:42 jiunming Exp $
     ********************************************************************

     This file is part of the XSL DocBook Stylesheet distribution.

     Copyright (C) 1999, 2000, 2001, 2002 Norman Walsh.
     ******************************************************************** -->

<!-- ==================================================================== -->

<!-- subtitle markup -->

<doc:mode mode="subtitle.markup" xmlns="">
<refpurpose>Provides access to element subtitles</refpurpose>
<refdescription>
<para>Processing an element in the
<literal role="mode">subtitle.markup</literal> mode produces the
subtitle of the element.
</para>
</refdescription>
</doc:mode>

<xsl:template match="*" mode="subtitle.markup">
  <xsl:message>
    <xsl:text>Request for subtitle of unexpected element: </xsl:text>
    <xsl:value-of select="name(.)"/>
  </xsl:message>
  <xsl:text>???SUBTITLE???</xsl:text>
</xsl:template>

<xsl:template match="subtitle" mode="subtitle.markup">
  <xsl:param name="allow-anchors" select="'0'"/>
  <xsl:apply-templates/>
</xsl:template>

<xsl:template match="set" mode="subtitle.markup">
  <xsl:param name="allow-anchors" select="'0'"/>
  <xsl:apply-templates select="(setinfo/subtitle|subtitle)[1]"
                       mode="subtitle.markup">
    <xsl:with-param name="allow-anchors" select="$allow-anchors"/>
  </xsl:apply-templates>
</xsl:template>

<xsl:template match="book" mode="subtitle.markup">
  <xsl:param name="allow-anchors" select="'0'"/>
  <xsl:apply-templates select="(bookinfo/subtitle|subtitle)[1]"
                       mode="subtitle.markup">
    <xsl:with-param name="allow-anchors" select="$allow-anchors"/>
  </xsl:apply-templates>
</xsl:template>

<xsl:template match="part" mode="subtitle.markup">
  <xsl:param name="allow-anchors" select="'0'"/>
  <xsl:apply-templates select="(partinfo/subtitle
                                |docinfo/subtitle
                                |subtitle)[1]"
                       mode="subtitle.markup">
    <xsl:with-param name="allow-anchors" select="$allow-anchors"/>
  </xsl:apply-templates>
</xsl:template>

<xsl:template match="preface|chapter|appendix" mode="subtitle.markup">
  <xsl:param name="allow-anchors" select="'0'"/>
  <xsl:apply-templates select="(docinfo/subtitle
                                |prefaceinfo/subtitle
                                |chapterinfo/subtitle
                                |appendixinfo/subtitle
                                |subtitle)[1]"
                       mode="subtitle.markup">
    <xsl:with-param name="allow-anchors" select="$allow-anchors"/>
  </xsl:apply-templates>
</xsl:template>

<xsl:template match="article" mode="subtitle.markup">
  <xsl:param name="allow-anchors" select="'0'"/>
  <xsl:apply-templates select="(artheader/subtitle
                                |articleinfo/subtitle
                                |subtitle)[1]"
                       mode="subtitle.markup">
    <xsl:with-param name="allow-anchors" select="$allow-anchors"/>
  </xsl:apply-templates>
</xsl:template>

<xsl:template match="dedication|colophon" mode="subtitle.markup">
  <xsl:param name="allow-anchors" select="'0'"/>
  <xsl:apply-templates select="subtitle"
                       mode="subtitle.markup">
    <xsl:with-param name="allow-anchors" select="$allow-anchors"/>
  </xsl:apply-templates>
</xsl:template>

<xsl:template match="reference" mode="subtitle.markup">
  <xsl:param name="allow-anchors" select="'0'"/>
  <xsl:apply-templates select="(referenceinfo/subtitle
                                |docinfo/subtitle
                                |subtitle)[1]"
                       mode="subtitle.markup">
    <xsl:with-param name="allow-anchors" select="$allow-anchors"/>
  </xsl:apply-templates>
</xsl:template>

<xsl:template match="refentry" mode="subtitle.markup">
  <xsl:param name="allow-anchors" select="'0'"/>
  <xsl:apply-templates select="(refentryinfo/subtitle
                                |docinfo/subtitle)[1]"
                       mode="subtitle.markup">
    <xsl:with-param name="allow-anchors" select="$allow-anchors"/>
  </xsl:apply-templates>
</xsl:template>

<xsl:template match="section
                     |sect1|sect2|sect3|sect4|sect5
                     |refsect1|refsect2|refsect3
                     |simplesect"
              mode="subtitle.markup">
  <xsl:param name="allow-anchors" select="'0'"/>
  <xsl:apply-templates select="(sectioninfo/subtitle
                                |sect1info/subtitle
                                |sect2info/subtitle
                                |sect3info/subtitle
                                |sect4info/subtitle
                                |sect5info/subtitle
                                |refsect1info/subtitle
                                |refsect2info/subtitle
                                |refsect3info/subtitle
                                |subtitle)[1]"
                       mode="subtitle.markup">
    <xsl:with-param name="allow-anchors" select="$allow-anchors"/>
  </xsl:apply-templates>
</xsl:template>

</xsl:stylesheet>


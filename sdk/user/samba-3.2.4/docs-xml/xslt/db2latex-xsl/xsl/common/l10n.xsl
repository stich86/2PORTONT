<?xml version='1.0'?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns:l="http://docbook.sourceforge.net/xmlns/l10n/1.0"
                exclude-result-prefixes="l"
                version='1.0'>

<!-- ********************************************************************
     $Id: l10n.xsl,v 1.1.1.1 2009/05/07 11:14:42 jiunming Exp $
     ********************************************************************

     This file is part of the XSL DocBook Stylesheet distribution.

     This file contains localization templates (for internationalization)

     Copyright (C) 1999, 2000, 2001, 2002 Norman Walsh.
     ******************************************************************** -->

<xsl:param name="l10n.xml" select="document('../common/l10n.xml')"/>
<xsl:param name="local.l10n.xml" select="document('')"/>
<xsl:param name="l10n.gentext.language" select="''"/>
<xsl:param name="l10n.gentext.default.language" select="'fr'"/>
<xsl:param name="l10n.gentext.use.xref.language" select="false()"/>

<xsl:template name="l10n.language">
  <xsl:param name="target" select="."/>
  <xsl:param name="xref-context" select="false()"/>

  <xsl:variable name="mc-language">
    <xsl:choose>
      <xsl:when test="$l10n.gentext.language != ''">
        <xsl:value-of select="$l10n.gentext.language"/>
      </xsl:when>

      <xsl:when test="$xref-context or $l10n.gentext.use.xref.language != 0">
        <!-- can't do this one step: attributes are unordered! -->
        <xsl:variable name="lang-scope"
                      select="($target/ancestor-or-self::*[@lang]
                               |$target/ancestor-or-self::*[@xml:lang])[last()]"/>
        <xsl:variable name="lang-attr"
                      select="($lang-scope/@lang | $lang-scope/@xml:lang)[1]"/>
        <xsl:choose>
          <xsl:when test="string($lang-attr) = ''">
            <xsl:value-of select="$l10n.gentext.default.language"/>
          </xsl:when>
          <xsl:otherwise>
            <xsl:value-of select="$lang-attr"/>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:when>

      <xsl:otherwise>
        <!-- can't do this one step: attributes are unordered! -->
        <xsl:variable name="lang-scope"
                      select="($target/ancestor-or-self::*[@lang]
                           |$target/ancestor-or-self::*[@xml:lang])[last()]"/>
        <xsl:variable name="lang-attr"
                      select="($lang-scope/@lang | $lang-scope/@xml:lang)[1]"/>

        <xsl:choose>
          <xsl:when test="string($lang-attr) = ''">
            <xsl:value-of select="$l10n.gentext.default.language"/>
          </xsl:when>
          <xsl:otherwise>
            <xsl:value-of select="$lang-attr"/>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:variable>

  <xsl:variable name="language" select="translate($mc-language,
                                        'ABCDEFGHIJKLMNOPQRSTUVWXYZ',
                                        'abcdefghijklmnopqrstuvwxyz')"/>

  <xsl:variable name="adjusted.language">
    <xsl:choose>
      <xsl:when test="contains($language,'-')">
        <xsl:value-of select="substring-before($language,'-')"/>
        <xsl:text>_</xsl:text>
        <xsl:value-of select="substring-after($language,'-')"/>
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="$language"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:variable>

  <xsl:choose>
    <xsl:when test="$l10n.xml/l:i18n/l:l10n[@language=$adjusted.language]">
      <xsl:value-of select="$adjusted.language"/>
    </xsl:when>
    <!-- try just the lang code without country -->
    <xsl:when test="$l10n.xml/l:i18n/l:l10n[@language=substring-before($adjusted.language,'_')]">
      <xsl:value-of select="substring-before($adjusted.language,'_')"/>
    </xsl:when>
    <!-- or use the default -->
    <xsl:otherwise>
      <xsl:message>
        <xsl:text>No localization exists for "</xsl:text>
        <xsl:value-of select="$adjusted.language"/>
        <xsl:text>" or "</xsl:text>
        <xsl:value-of select="substring-before($adjusted.language,'_')"/>
        <xsl:text>". Using default "</xsl:text>
        <xsl:value-of select="$l10n.gentext.default.language"/>
        <xsl:text>".</xsl:text>
      </xsl:message>
      <xsl:value-of select="$l10n.gentext.default.language"/>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template name="language.attribute">
  <xsl:param name="node" select="."/>

  <xsl:variable name="language">
    <xsl:choose>
      <xsl:when test="$l10n.gentext.language != ''">
        <xsl:value-of select="$l10n.gentext.language"/>
      </xsl:when>

      <xsl:otherwise>
        <!-- can't do this one step: attributes are unordered! -->
        <xsl:variable name="lang-scope"
                      select="($node/ancestor-or-self::*[@lang]
                               |$node/ancestor-or-self::*[@xml:lang])[last()]"/>
        <xsl:variable name="lang-attr"
                      select="($lang-scope/@lang | $lang-scope/@xml:lang)[1]"/>

        <xsl:choose>
          <xsl:when test="string($lang-attr) = ''">
            <xsl:value-of select="$l10n.gentext.default.language"/>
          </xsl:when>
          <xsl:otherwise>
            <xsl:value-of select="$lang-attr"/>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:variable>

  <xsl:if test="$language != ''">
    <xsl:attribute name="lang">
      <xsl:value-of select="$language"/>
    </xsl:attribute>
  </xsl:if>
</xsl:template>

<xsl:template name="gentext">
  <xsl:param name="key" select="local-name(.)"/>
  <xsl:param name="lang">
    <xsl:call-template name="l10n.language"/>
  </xsl:param>

  <xsl:variable name="local.l10n.gentext"
                select="($local.l10n.xml//l:i18n/l:l10n[@language=$lang]/l:gentext[@key=$key])[1]"/>

  <xsl:variable name="l10n.gentext"
                select="($l10n.xml/l:i18n/l:l10n[@language=$lang]/l:gentext[@key=$key])[1]"/>

  <xsl:choose>
    <xsl:when test="count($local.l10n.gentext) &gt; 0">
      <xsl:value-of select="$local.l10n.gentext/@text"/>
    </xsl:when>
    <xsl:when test="count($l10n.gentext) &gt; 0">
      <xsl:value-of select="$l10n.gentext/@text"/>
    </xsl:when>
    <xsl:otherwise>
      <xsl:message>
        <xsl:text>No "</xsl:text>
        <xsl:value-of select="$lang"/>
        <xsl:text>" localization of "</xsl:text>
        <xsl:value-of select="$key"/>
        <xsl:text>" exists</xsl:text>
	<xsl:choose>
	  <xsl:when test="$lang = 'en'">
	     <xsl:text>.</xsl:text>
	  </xsl:when>
	  <xsl:otherwise>
	     <xsl:text>; using "en".</xsl:text>
	  </xsl:otherwise>
	</xsl:choose>
      </xsl:message>

      <xsl:value-of select="($l10n.xml/l:i18n/l:l10n[@language='en']/l:gentext[@key=$key])[1]/@text"/>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template name="gentext.element.name">
  <xsl:param name="element.name" select="name(.)"/>
  <xsl:param name="lang">
    <xsl:call-template name="l10n.language"/>
  </xsl:param>

  <xsl:call-template name="gentext">
    <xsl:with-param name="key" select="$element.name"/>
    <xsl:with-param name="lang" select="$lang"/>
  </xsl:call-template>
</xsl:template>

<xsl:template name="gentext.space">
  <xsl:text> </xsl:text>
</xsl:template>

<xsl:template name="gentext.edited.by">
  <xsl:call-template name="gentext">
    <xsl:with-param name="key" select="'Editedby'"/>
  </xsl:call-template>
</xsl:template>

<xsl:template name="gentext.by">
  <xsl:call-template name="gentext">
    <xsl:with-param name="key" select="'by'"/>
  </xsl:call-template>
</xsl:template>

<xsl:template name="gentext.dingbat">
  <xsl:param name="dingbat">bullet</xsl:param>
  <xsl:param name="lang">
    <xsl:call-template name="l10n.language"/>
  </xsl:param>

  <xsl:variable name="local.l10n.dingbat"
                select="($local.l10n.xml//l:i18n/l:l10n[@language=$lang]/l:dingbat[@key=$dingbat])[1]"/>

  <xsl:variable name="l10n.dingbat"
                select="($l10n.xml/l:i18n/l:l10n[@language=$lang]/l:dingbat[@key=$dingbat])[1]"/>

  <xsl:choose>
    <xsl:when test="count($local.l10n.dingbat) &gt; 0">
      <xsl:value-of select="$local.l10n.dingbat/@text"/>
    </xsl:when>
    <xsl:when test="count($l10n.dingbat) &gt; 0">
      <xsl:value-of select="$l10n.dingbat/@text"/>
    </xsl:when>
    <xsl:otherwise>
      <xsl:message>
        <xsl:text>No "</xsl:text>
        <xsl:value-of select="$lang"/>
        <xsl:text>" localization of dingbat </xsl:text>
        <xsl:value-of select="$dingbat"/>
        <xsl:text> exists; using "en".</xsl:text>
      </xsl:message>

      <xsl:value-of select="($l10n.xml/l:i18n/l:l10n[@language='en']/l:dingbat[@key=$dingbat])[1]/@text"/>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template name="gentext.startquote">
  <xsl:call-template name="gentext.dingbat">
    <xsl:with-param name="dingbat">startquote</xsl:with-param>
  </xsl:call-template>
</xsl:template>

<xsl:template name="gentext.endquote">
  <xsl:call-template name="gentext.dingbat">
    <xsl:with-param name="dingbat">endquote</xsl:with-param>
  </xsl:call-template>
</xsl:template>

<xsl:template name="gentext.nestedstartquote">
  <xsl:call-template name="gentext.dingbat">
    <xsl:with-param name="dingbat">nestedstartquote</xsl:with-param>
  </xsl:call-template>
</xsl:template>

<xsl:template name="gentext.nestedendquote">
  <xsl:call-template name="gentext.dingbat">
    <xsl:with-param name="dingbat">nestedendquote</xsl:with-param>
  </xsl:call-template>
</xsl:template>

<xsl:template name="gentext.nav.prev">
  <xsl:call-template name="gentext">
    <xsl:with-param name="key" select="'nav-prev'"/>
  </xsl:call-template>
</xsl:template>

<xsl:template name="gentext.nav.next">
  <xsl:call-template name="gentext">
    <xsl:with-param name="key" select="'nav-next'"/>
  </xsl:call-template>
</xsl:template>

<xsl:template name="gentext.nav.home">
  <xsl:call-template name="gentext">
    <xsl:with-param name="key" select="'nav-home'"/>
  </xsl:call-template>
</xsl:template>

<xsl:template name="gentext.nav.up">
  <xsl:call-template name="gentext">
    <xsl:with-param name="key" select="'nav-up'"/>
  </xsl:call-template>
</xsl:template>

<!-- ============================================================ -->

<xsl:template name="gentext.template">
  <xsl:param name="context" select="'default'"/>
  <xsl:param name="name" select="'default'"/>
  <xsl:param name="origname" select="$name"/>
  <xsl:param name="purpose"/>
  <xsl:param name="xrefstyle"/>
  <xsl:param name="referrer"/>
  <xsl:param name="lang">
    <xsl:call-template name="l10n.language"/>
  </xsl:param>

  <xsl:variable name="local.localization.node"
                select="($local.l10n.xml//l:i18n/l:l10n[@language=$lang])[1]"/>

  <xsl:variable name="localization.node"
                select="($l10n.xml/l:i18n/l:l10n[@language=$lang])[1]"/>

  <xsl:if test="count($localization.node) = 0
                and count($local.localization.node) = 0">
    <xsl:message>
      <xsl:text>No "</xsl:text>
      <xsl:value-of select="$lang"/>
      <xsl:text>" localization exists.</xsl:text>
    </xsl:message>
  </xsl:if>

  <xsl:variable name="local.context.node"
                select="$local.localization.node/l:context[@name=$context]"/>

  <xsl:variable name="context.node"
                select="$localization.node/l:context[@name=$context]"/>

  <xsl:variable name="en.context.node"
                select="$l10n.xml/l:i18n/l:l10n[@language='en']/l:context[@name=$context]"/>

  <xsl:if test="count($context.node) = 0
                and count($local.context.node) = 0">
    <xsl:message>
      <xsl:text>No context named "</xsl:text>
      <xsl:value-of select="$context"/>
      <xsl:text>" exists in the "</xsl:text>
      <xsl:value-of select="$lang"/>
      <xsl:text>" localization; trying "en".</xsl:text>
    </xsl:message>
  </xsl:if>

  <xsl:variable name="local.template.node"
                select="($local.context.node/l:template[@name=$name
                                                        and @style
                                                        and @style=$xrefstyle]
                        |$local.context.node/l:template[@name=$name
                                                        and not(@style)])[1]"/>

  <xsl:variable name="template.node"
                select="($context.node/l:template[@name=$name
                                                  and @style
                                                  and @style=$xrefstyle]
                        |$context.node/l:template[@name=$name
                                                  and not(@style)])[1]"/>

  <xsl:variable name="en.template.node"
                select="($en.context.node/l:template[@name=$name
                                                  and @style
                                                  and @style=$xrefstyle]
                        |$en.context.node/l:template[@name=$name
                                                  and not(@style)])[1]"/>

  <xsl:choose>
    <xsl:when test="$local.template.node/@text">
      <xsl:value-of select="$local.template.node/@text"/>
    </xsl:when>
    <xsl:when test="$template.node/@text">
      <xsl:value-of select="$template.node/@text"/>
    </xsl:when>
    <xsl:when test="$en.template.node/@text">
      <xsl:value-of select="$en.template.node/@text"/>
    </xsl:when>
    <xsl:otherwise>
      <xsl:choose>
        <xsl:when test="contains($name, '/')">
          <xsl:call-template name="gentext.template">
            <xsl:with-param name="context" select="$context"/>
            <xsl:with-param name="name" select="substring-after($name, '/')"/>
            <xsl:with-param name="origname" select="$origname"/>
            <xsl:with-param name="purpose" select="$purpose"/>
            <xsl:with-param name="xrefstyle" select="$xrefstyle"/>
            <xsl:with-param name="referrer" select="$referrer"/>
            <xsl:with-param name="lang" select="$lang"/>
          </xsl:call-template>
        </xsl:when>
        <xsl:otherwise>
          <xsl:message>
            <xsl:text>No template for "</xsl:text>
            <xsl:value-of select="$origname"/>
            <xsl:text>" (or any of its leaves) exists
in the context named "</xsl:text>
            <xsl:value-of select="$context"/>
            <xsl:text>" in the "</xsl:text>
            <xsl:value-of select="$lang"/>
            <xsl:text>" localization.</xsl:text>
          </xsl:message>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template name="gentext.template.exists">
  <xsl:param name="context" select="'default'"/>
  <xsl:param name="name" select="'default'"/>
  <xsl:param name="origname" select="$name"/>
  <xsl:param name="purpose"/>
  <xsl:param name="xrefstyle"/>
  <xsl:param name="referrer"/>
  <xsl:param name="lang">
    <xsl:call-template name="l10n.language"/>
  </xsl:param>

  <xsl:variable name="local.localization.node"
                select="($local.l10n.xml//l:i18n/l:l10n[@language=$lang])[1]"/>

  <xsl:variable name="localization.node"
                select="($l10n.xml/l:i18n/l:l10n[@language=$lang])[1]"/>

  <xsl:variable name="local.context.node"
                select="$local.localization.node/l:context[@name=$context]"/>

  <xsl:variable name="context.node"
                select="$localization.node/l:context[@name=$context]"/>

  <xsl:variable name="local.template.node"
                select="($local.context.node/l:template[@name=$name
                                                        and @style
                                                        and @style=$xrefstyle]
                        |$local.context.node/l:template[@name=$name
                                                        and not(@style)])[1]"/>

  <xsl:variable name="template.node"
                select="($context.node/l:template[@name=$name
                                                  and @style
                                                  and @style=$xrefstyle]
                        |$context.node/l:template[@name=$name
                                                  and not(@style)])[1]"/>

  <xsl:choose>
    <xsl:when test="$local.template.node/@text">1</xsl:when>
    <xsl:when test="$template.node/@text">1</xsl:when>
    <xsl:otherwise>
      <xsl:choose>
        <xsl:when test="contains($name, '/')">
          <xsl:call-template name="gentext.template.exists">
            <xsl:with-param name="context" select="$context"/>
            <xsl:with-param name="name" select="substring-after($name, '/')"/>
            <xsl:with-param name="origname" select="$origname"/>
            <xsl:with-param name="purpose" select="$purpose"/>
            <xsl:with-param name="xrefstyle" select="$xrefstyle"/>
            <xsl:with-param name="referrer" select="$referrer"/>
            <xsl:with-param name="lang" select="$lang"/>
          </xsl:call-template>
        </xsl:when>
        <xsl:otherwise>0</xsl:otherwise>
      </xsl:choose>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

</xsl:stylesheet>


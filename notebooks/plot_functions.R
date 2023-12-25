library(ggplot2)

despine <- theme(panel.border = element_blank(), panel.grid.major = element_blank(),
    panel.grid.minor = element_blank(), axis.line = element_line(colour = "black"),
                text=element_text(size=17)) 

plot_sfs <- function(graph, rewards) {
    sfs <- sapply(1:(dim(rewards)[1]-1), function(i) expectation(graph, rewards[i,]))
    data.frame(
      ton=seq(1,length(sfs)),  
      brlen=sfs
      ) %>% ggplot(aes(x=ton, y=brlen, fill=ton)) + 
      geom_bar(stat = "identity", width=0.8) + scale_fill_viridis() + 
      despine
}

plot_sfs_dph <- function(graph, rewards, trunc=4) {
    result = data.frame()
    for (i in 1:(nrow(rewards)-1)) {
        x <- seq(from = 0, to = trunc, by = 0.01)
        pdf <- dph(x, reward_transform(graph, rewards[i, ]))
        df <- data.frame(prob = pdf, t=x, ton=i)
        result <- rbind(result, df)
    }
    result %>% ggplot(aes(y=prob, x=t, group=ton, color=ton)) +
        geom_line(linewidth=1) + scale_color_viridis() + despine
}

get_exp_mat <- function(graph, rewards) {
    s <- nrow(rewards)
    exp_mat <- matrix(nrow=s+1,ncol=s+1)
    for (i in 0:s) {
      for (j in 0:s) {
        exp_mat[i+1,j+1] <- expectation(graph, rewards[props_to_index(s, i, j, 1),] + rewards[props_to_index(s, i, j, 2),])
      }
    } 
    return(exp_mat)
}
                                  
plot_exp_mat <- function(exp_mat) {  
    df <- as.data.frame(exp_mat) #%>% gather()
    df <- df %>% rownames_to_column('ton1') %>% gather('ton2', 'value', -c(ton1))

    limit <- max(abs(df$value)) * c(-1, 1)
    
    ggplot(df, aes(ton1, ton2)) +
        geom_tile(aes(fill = value)) + 
        geom_text(aes(label = round(value, 2))) +
    scale_x_discrete(labels= seq(0, nrow(exp_mat))) + 
    scale_y_discrete(labels= seq(0, nrow(exp_mat))) + 
    scale_fill_distiller(palette = 'PiYG',direction = 1,
                        limit=limit) +
    theme_minimal() +
     theme(panel.grid.major = element_blank(), 
            panel.grid.minor = element_blank(), 
            text=element_text(size=17))

}

get_cov_mat <- function(graph, rewards) {
    n <- nrow(rewards)
    cov_mat <- matrix(nrow=n-1,ncol=n-1)
    for (i in 1:(n-1)) {
        for (j in 1:(n-1)) {
            cov_mat[i, j] <- covariance(graph, rewards[i,], rewards[j,])
        }
    }
    return(cov_mat)
}
                  
plot_cov_mat <- function(cov_mat) {
   
    df <- as.data.frame(cov_mat)
    df <- df %>% rownames_to_column('ton1') %>% gather('ton2', 'value', -c(ton1))
    
    ggplot(df, aes(ton1, ton2)) +
        geom_tile(aes(fill = value)) + 
        scale_y_discrete(labels= seq(1, nrow(cov_mat))) + 
        scale_fill_continuous(type = "viridis") + 
        theme_minimal() + 
        theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank(), text=element_text(size=17)) 

}


plot_graph <- function(gam, constrained=TRUE, size=c(6, 6), fontsize=10, rankdir="LR") {

    if (constrained) {
        constrained <- 'true'
    } else {
        constrained <- 'false'
    }

    edge_templ <- '"FROM" -> "TO" [constraint=true, label="LABEL",labelfloat=false];'
    start_name <- 'IPV'
    absorbing_name <- 'Absorb'
    edges <- c()
    for (i in 1:length(gam$IPV)) {
        if (gam$IPV[i] > 0) {
            edge <- edge_templ
            edge <- sub('FROM', start_name, edge)
            edge <- sub('TO', paste(gam$states[i,], collapse = ","), edge)
            edge <- sub('LABEL', gam$SIM[i, 1], edge)
            edges <- c(edges, edge)
        }
    }    
    for (i in 1:nrow(gam$states)) {
        for (j in 1:nrow(gam$states)) {
            if ((i != j) && (gam$SIM[i, j] > 0)) {
                edge <- edge_templ
                edge <- sub('FROM', paste(gam$states[i,], collapse = ","), edge)
                edge <- sub('TO', paste(gam$states[j,], collapse = ","), edge)
                edge <- sub('LABEL', gam$SIM[i, j], edge)
                edges <- c(edges, edge)
            }
        }
    }
    absorb_rates <- -rowSums(gam$SIM)
    for (i in 1:nrow(gam$states)) {
        if (absorb_rates[i] > 0) {
            # edges <- c(edges, paste('"', from, '"', ' -> ', 'Absorb', '[constraint=true, label="', label, '",labelfloat=false]', ';', sep='')) 
            edge <- edge_templ
            edge <- sub('FROM', paste(gam$states[i,], collapse = ","), edge)
            edge <- sub('TO', absorbing_name, edge)
            edge <- sub('LABEL', absorb_rates[i], edge)
            edges <- c(edges, edge)
        }
    }
    graph_spec <- paste(edges, sep='\n', collapse = '')
    style_str <- '
        rankdir=RANKDIR;
        size="SIZEX,SIZEY";
        fontname="Helvetica,Arial,sans-serif"
    	node [fontname="Helvetica,Arial,sans-serif", fontsize=FONTSIZE]
    	edge [fontname="Helvetica,Arial,sans-serif", fontsize=FONTSIZE]
        Absorb [style=filled,color="lightgrey"]
        IPV [style=filled,color="lightgrey"]
    '
    style_str <- sub('SIZEX', size[1], style_str)
    style_str <- sub('SIZEY', size[2], style_str)
    style_str <- gsub('FONTSIZE', fontsize, style_str)    
    style_str <- gsub('RANKDIR', rankdir, style_str)    
    graph_string <- paste('digraph G {', style_str, graph_spec, '}', sep='\n')
    system("dot -Tsvg -o tmp.svg", input=graph_string, intern=TRUE)
    return(display_svg(file="tmp.svg"))
    }